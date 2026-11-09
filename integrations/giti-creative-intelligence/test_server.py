import json
import os
import subprocess
import sys
import time
import unittest
import urllib.error
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent

class GatewaySmokeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.port = 18765
        env = os.environ.copy()
        env.update({"GITI_CREATIVE_HOST": "127.0.0.1",
                    "GITI_CREATIVE_PORT": str(cls.port),
                    "PIXVERSE_API_KEY": "",
                    "PIXVERSE_FREE_CREDITS_BUDGET": "0"})
        cls.proc = subprocess.Popen([sys.executable, str(ROOT / "server.py")],
            cwd=str(ROOT), env=env, stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL)
        cls.base = "http://127.0.0.1:%s" % cls.port
        for _ in range(40):
            try:
                urllib.request.urlopen(cls.base + "/health", timeout=0.2)
                break
            except Exception:
                time.sleep(0.1)
        else:
            raise RuntimeError("Gateway did not start")

    @classmethod
    def tearDownClass(cls):
        cls.proc.terminate()
        cls.proc.wait(timeout=5)

    def get_json(self, path):
        with urllib.request.urlopen(self.base + path, timeout=2) as response:
            return json.loads(response.read())

    def post_json(self, path, body):
        req = urllib.request.Request(self.base + path,
            data=json.dumps(body).encode(),
            headers={"Content-Type": "application/json"}, method="POST")
        with urllib.request.urlopen(req, timeout=2) as response:
            return json.loads(response.read())

    def test_health_locked_by_default(self):
        data = self.get_json("/health")
        self.assertTrue(data["free_only"])
        self.assertFalse(data["generation_enabled"])
        self.assertEqual(data["free_budget_remaining"], 0)

    def test_capabilities_are_honest(self):
        data = self.get_json("/v1/capabilities")
        self.assertTrue(data["implemented"]["video_text_to_video"])
        self.assertFalse(data["implemented"]["image_generation"])
        self.assertFalse(data["implemented"]["music_generation"])

    def test_preview_does_not_submit(self):
        data = self.post_json("/v1/jobs/preview", {
            "prompt": "Persian cyberpunk skyline",
            "duration": 5, "aspect_ratio": "9:16"})
        self.assertFalse(data["submitted"])
        self.assertFalse(data["submission_allowed"])
        self.assertEqual(data["estimated_credits"], 45)

    def test_invalid_duration_rejected(self):
        with self.assertRaises(urllib.error.HTTPError) as error:
            self.post_json("/v1/jobs/preview", {
                "prompt": "test", "duration": 999})
        self.assertEqual(error.exception.code, 400)

if __name__ == "__main__":
    unittest.main()
