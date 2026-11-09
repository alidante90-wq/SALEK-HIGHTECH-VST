#!/usr/bin/env python3
"""GITI Creative Intelligence local gateway. Python 3.10+, standard library only."""
import json, os, threading, time, uuid
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parent
ENV_FILE = ROOT / ".env"
STATE_FILE = ROOT / "data" / "credit_state.json"
STATE_LOCK = threading.Lock()
MAX_BODY = 64 * 1024
RATIOS = {"16:9", "9:16", "1:1", "4:3", "3:4", "21:9"}
API_KEY = ""
API_BASE = "https://app-api.pixverse.ai"
HOST = "127.0.0.1"
PORT = 8765
FREE_BUDGET = 0
CREDITS_PER_SECOND = 9  # Published PixVerse V6 / 720p / no-audio rate.

def load_env_file():
    if not ENV_FILE.exists():
        return
    for raw in ENV_FILE.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        os.environ.setdefault(key.strip(), value.strip().strip('"').strip("'"))

def save_state(state):
    STATE_FILE.parent.mkdir(parents=True, exist_ok=True)
    tmp = STATE_FILE.with_suffix(".tmp")
    tmp.write_text(json.dumps(state, indent=2), encoding="utf-8")
    tmp.replace(STATE_FILE)

def load_state():
    STATE_FILE.parent.mkdir(parents=True, exist_ok=True)
    if STATE_FILE.exists():
        try:
            value = json.loads(STATE_FILE.read_text(encoding="utf-8"))
            if isinstance(value, dict) and isinstance(value.get("remaining"), int):
                return value
        except (OSError, json.JSONDecodeError):
            pass
    value = {"remaining": FREE_BUDGET, "initial_budget": FREE_BUDGET,
             "updated_at": int(time.time())}
    save_state(value)
    return value

def provider_request(method, path, payload=None):
    if not API_KEY:
        raise RuntimeError("PIXVERSE_API_KEY is not configured")
    trace = str(uuid.uuid4())
    data = None if payload is None else json.dumps(payload).encode("utf-8")
    headers = {"API-KEY": API_KEY, "Ai-trace-id": trace}
    if data is not None:
        headers["Content-Type"] = "application/json"
    req = Request(API_BASE + path, data=data, headers=headers, method=method)
    try:
        with urlopen(req, timeout=45) as response:
            result = json.loads(response.read().decode("utf-8"))
    except HTTPError as exc:
        raise RuntimeError("PixVerse HTTP %s: %s" % (
            exc.code, exc.read(2048).decode("utf-8", errors="replace"))) from exc
    except (URLError, TimeoutError) as exc:
        raise RuntimeError("Transport outcome uncertain; reconcile before retry: %s" % exc) from exc
    if not isinstance(result, dict):
        raise RuntimeError("Unexpected PixVerse response")
    result["_trace_id"] = trace
    return result

def parse_video(body):
    prompt = body.get("prompt")
    duration = body.get("duration", 5)
    ratio = body.get("aspect_ratio", "16:9")
    if not isinstance(prompt, str) or not prompt.strip() or len(prompt) > 2000:
        raise ValueError("prompt must contain 1-2000 characters")
    if not isinstance(duration, int) or isinstance(duration, bool) or not 1 <= duration <= 15:
        raise ValueError("duration must be an integer from 1 to 15")
    if ratio not in RATIOS:
        raise ValueError("unsupported aspect_ratio")
    return prompt.strip(), duration, ratio

class Handler(BaseHTTPRequestHandler):
    server_version = "GITI-Creative/0.1"
    def log_message(self, fmt, *args):
        # Never log prompt bodies, API keys, or request headers.
        super().log_message(fmt, *args)
    def send_json(self, code, value):
        data = json.dumps(value, ensure_ascii=False).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)
    def read_body(self):
        length = int(self.headers.get("Content-Length", "0"))
        if length < 1 or length > MAX_BODY:
            raise ValueError("body must be between 1 byte and 64 KiB")
        body = json.loads(self.rfile.read(length).decode("utf-8"))
        if not isinstance(body, dict):
            raise ValueError("JSON body must be an object")
        return body
    def do_GET(self):
        if self.path == "/health":
            with STATE_LOCK:
                state = load_state()
            self.send_json(200, {
                "service": "GITI CREATIVE INTELLIGENCE", "version": "0.1.0",
                "provider": "pixverse", "free_only": True,
                "api_key_configured": bool(API_KEY),
                "free_budget_remaining": state["remaining"],
                "generation_enabled": bool(API_KEY and state["remaining"] > 0)
            })
            return
        if self.path == "/v1/capabilities":
            self.send_json(200, {
                "provider": "pixverse", "free_only": True,
                "implemented": {"video_text_to_video": True, "video_task_status": True,
                    "image_generation": False, "image_to_video": False,
                    "music_generation": False, "voice_generation": False,
                    "video_editing": False},
                "planned": ["image", "image-to-video", "music", "voice", "editing",
                    "templates", "motion-control", "provider-routing", "asset-library"],
                "video_limits": {"model": "v6", "quality": "720p",
                    "audio": False, "max_duration_seconds": 15}
            })
            return
        parts = self.path.strip("/").split("/")
        if len(parts) == 3 and parts[:2] == ["v1", "jobs"] and parts[2].isdigit():
            try:
                result = provider_request("GET", "/openapi/v2/video/result/" + parts[2])
                if result.get("ErrCode") != 0:
                    self.send_json(502, {"error": "pixverse_error", "provider_response": result})
                    return
                resp = result.get("Resp") or {}
                self.send_json(200, {"provider": "pixverse", "video_id": parts[2],
                    "status": resp.get("status"), "url": resp.get("url"),
                    "credits": resp.get("credits"), "trace_id": result.get("_trace_id")})
            except RuntimeError as exc:
                self.send_json(502, {"error": "provider_unavailable", "detail": str(exc)})
            return
        self.send_json(404, {"error": "not_found"})
    def do_POST(self):
        if self.path not in ("/v1/jobs/preview", "/v1/jobs/video"):
            self.send_json(404, {"error": "not_found"})
            return
        try:
            prompt, duration, ratio = parse_video(self.read_body())
        except (ValueError, json.JSONDecodeError, UnicodeDecodeError) as exc:
            self.send_json(400, {"error": "invalid_request", "detail": str(exc)})
            return
        estimate = duration * CREDITS_PER_SECOND
        with STATE_LOCK:
            state = load_state()
            remaining = state["remaining"]
        info = {"provider": "pixverse", "mode": "text_to_video", "model": "v6",
            "quality": "720p", "audio": False, "duration_seconds": duration,
            "aspect_ratio": ratio, "estimated_credits": estimate,
            "free_credits_remaining": remaining, "free_only": True}
        if self.path.endswith("/preview"):
            allowed = bool(API_KEY and remaining >= estimate)
            reason = None if allowed else ("api_key_missing" if not API_KEY
                else "insufficient_verified_free_credits")
            self.send_json(200, {**info, "submission_allowed": allowed,
                "blocked_reason": reason, "submitted": False})
            return
        if not API_KEY:
            self.send_json(503, {**info, "error": "api_key_missing",
                "message": "No provider request was sent."})
            return
        with STATE_LOCK:
            state = load_state()
            if state["remaining"] < estimate:
                self.send_json(402, {**info, "error": "insufficient_verified_free_credits",
                    "message": "No provider request was sent."})
                return
            state["remaining"] -= estimate
            state["updated_at"] = int(time.time())
            save_state(state)
        try:
            result = provider_request("POST", "/openapi/v2/video/text/generate", {
                "model": "v6", "prompt": prompt, "duration": duration,
                "quality": "720p", "aspect_ratio": ratio})
        except RuntimeError as exc:
            # Keep reservation on ambiguous transport failure; never auto-retry.
            self.send_json(502, {**info, "error": "submission_uncertain",
                "detail": str(exc), "message": "Reservation retained. Reconcile task history before retrying."})
            return
        if result.get("ErrCode") != 0:
            with STATE_LOCK:
                state = load_state()
                state["remaining"] += estimate
                state["updated_at"] = int(time.time())
                save_state(state)
            self.send_json(502, {"error": "pixverse_rejected_request",
                "provider_response": result})
            return
        resp = result.get("Resp") or {}
        actual = resp.get("credits")
        if isinstance(actual, int):
            with STATE_LOCK:
                state = load_state()
                state["remaining"] = max(0, state["remaining"] + estimate - actual)
                state["updated_at"] = int(time.time())
                save_state(state)
        video_id = resp.get("video_id")
        self.send_json(202, {"provider": "pixverse", "video_id": video_id,
            "status": "submitted", "credits_estimated": estimate,
            "credits_reported": actual, "trace_id": result.get("_trace_id"),
            "status_endpoint": ("/v1/jobs/" + str(video_id)) if video_id else None})

def main():
    load_env_file()
    global API_KEY, API_BASE, HOST, PORT, FREE_BUDGET
    API_KEY = os.getenv("PIXVERSE_API_KEY", "").strip()
    API_BASE = os.getenv("PIXVERSE_API_BASE_URL", "https://app-api.pixverse.ai").rstrip("/")
    FREE_BUDGET = max(0, int(os.getenv("PIXVERSE_FREE_CREDITS_BUDGET", "0")))
    HOST = os.getenv("GITI_CREATIVE_HOST", "127.0.0.1")
    PORT = int(os.getenv("GITI_CREATIVE_PORT", "8765"))
    with STATE_LOCK:
        if not STATE_FILE.exists():
            load_state()
    print("GITI Creative Intelligence: http://%s:%s" % (HOST, PORT))
    print("FREE-ONLY MODE. Generation remains locked without verified free credits.")
    ThreadingHTTPServer((HOST, PORT), Handler).serve_forever()

if __name__ == "__main__":
    main()
