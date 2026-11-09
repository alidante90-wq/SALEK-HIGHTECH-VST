# GITI CREATIVE INTELLIGENCE — GENESIS 01

A separate creative-services layer for GITI desktop and the ISATIS website.
It does not modify the JUCE audio engine, DSP, presets, VST3/Standalone targets,
or existing interface.

## MVP scope

- Local Python 3.10+ HTTP gateway; standard library only.
- PixVerse V6 text-to-video submission and task-status retrieval.
- Preview/estimate endpoint that never contacts PixVerse.
- Strict free-only credit gate; default verified-free budget is zero.
- Server-side API key only; never expose it in the plugin or browser.
- Provider-neutral HTTP contract for future image, music, voice, editing,
  template, motion-control, and alternative-provider modules.

The other media modules are explicitly marked planned, not implemented.

## Run on Windows

PowerShell:

    cd integrations/giti-creative-intelligence
    Copy-Item .env.example .env
    .\start.ps1

Python 3.10+ is required. No pip installation is needed. The service binds to
127.0.0.1:8765 by default.

Endpoints:
- GET /health
- GET /v1/capabilities
- POST /v1/jobs/preview
- POST /v1/jobs/video
- GET /v1/jobs/{video_id}

## Safe smoke test

    $body = @{
      prompt = "A luminous Persian cyberpunk city at night, cinematic camera move"
      duration = 5
      aspect_ratio = "9:16"
    } | ConvertTo-Json
    Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8765/v1/jobs/preview -ContentType "application/json" -Body $body

Preview only estimates cost. It never submits a generation task.

## Free-only activation

1. Create an API key in the PixVerse API Platform.
2. Verify an actual free API credit allowance in the API billing console.
3. Put the API key in the local .env file.
4. Set PIXVERSE_FREE_CREDITS_BUDGET to that verified free allowance only.
   Never count purchased/package credits as free.
5. If the service has already created data/credit_state.json with a zero
   allowance, stop the service and remove that state file only before the
   first real submission; the next start initializes the verified allowance.

The service reserves estimated credits before submission. A transport timeout
is treated as uncertain and the reservation is retained to prevent duplicate
spending. Reconcile PixVerse task history and billing before manually adjusting
the local budget.

MVP video settings: V6, 720p, no generated audio, up to 15 seconds. The estimate
uses the published 9-credit/second rate. The provider response and billing
ledger are authoritative; if the returned debit differs, the local allowance
is reconciled conservatively.

## Integration boundaries

- GITI desktop calls this gateway from a background worker, never from the
  realtime audio callback.
- ISATIS calls the same gateway server-side over HTTPS after authentication.
- PixVerse credentials remain in a trusted backend environment.
- Before public deployment add user auth, per-user quotas, HTTPS, durable
  database-backed jobs, webhook signature verification, and abuse controls.

## Official references

- https://docs.platform.pixverse.ai/start-here-2394989m0
- https://docs.platform.pixverse.ai/text-to-video-generation-13016634e0
- https://docs.platform.pixverse.ai/query-task-status-43051212e0
- https://docs.platform.pixverse.ai/pixverse-api-pricing-2394995m0
