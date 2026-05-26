import json
import os
import urllib.request


def active_provider() -> str:
    return os.environ.get("ACTIVE_PROVIDER", "deepseek").strip().lower()


def provider_api_key() -> str:
    api_key = os.environ.get("PROVIDER_API_KEY", "").strip()
    if not api_key:
        raise RuntimeError("Missing environment variable PROVIDER_API_KEY.")
    if not api_key.isascii():
        raise RuntimeError("PROVIDER_API_KEY contains non-ASCII characters.")
    if " " in api_key:
        raise RuntimeError("PROVIDER_API_KEY contains spaces.")
    return api_key


def provider_model(default_model: str) -> str:
    model = os.environ.get("PROVIDER_MODEL", default_model).strip()
    if not model.isascii():
        raise RuntimeError("PROVIDER_MODEL contains non-ASCII characters.")
    return model


def provider_base_url(default_base_url: str) -> str:
    return os.environ.get("PROVIDER_BASE_URL", default_base_url).strip().rstrip("/")


def call_deepseek(message: str) -> str:
    api_key = provider_api_key()
    model = provider_model("deepseek-v4-flash")
    base_url = provider_base_url("https://api.deepseek.com")
    url = f"{base_url}/chat/completions"

    payload = {
        "model": model,
        "messages": [
            {
                "role": "system",
                "content": "You are the engineering assistant for SimAgent. Keep answers clear and implementation-focused.",
            },
            {
                "role": "user",
                "content": message,
            },
        ],
        "stream": False,
    }

    data = json.dumps(payload, ensure_ascii=False).encode("utf-8")
    request = urllib.request.Request(
        url,
        data=data,
        method="POST",
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        },
    )

    with urllib.request.urlopen(request, timeout=60) as response:
        response_body = response.read().decode("utf-8")

    result = json.loads(response_body)
    choices = result.get("choices", [])
    if not choices:
        raise RuntimeError("DeepSeek returned no choices.")

    reply = choices[0].get("message", {}).get("content", "").strip()
    if not reply:
        raise RuntimeError("DeepSeek returned empty text.")

    return reply


def call_provider(message: str, task: dict | None = None) -> str:
    if task:
        message = (
            f"{message}\n\n"
            "Engineering context:\n"
            f"- Task id: {task['task_id']}\n"
            f"- Task directory: {task['task_dir']}\n"
            f"- AppFlow manifest path: {task['manifest_path']}\n"
        )

    provider = active_provider()
    if provider == "deepseek":
        return call_deepseek(message)

    if provider == "openai":
        raise RuntimeError("Provider openai is configured but not implemented yet.")

    if provider == "gemini":
        raise RuntimeError("Provider gemini is configured but not implemented yet.")

    raise RuntimeError(f"Unsupported provider: {provider}")
