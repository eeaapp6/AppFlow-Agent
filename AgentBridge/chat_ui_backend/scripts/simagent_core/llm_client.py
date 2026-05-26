import json
import os
import urllib.request

from .config import Config
from .utils import extract_json_object


class DeepSeekClient:
    def __init__(self, config: Config | None = None):
        self._config = config or Config.from_environment()

    def chat(self, messages: list[dict]) -> str:
        api_key = os.environ.get("PROVIDER_API_KEY", "").strip()
        if not api_key:
            raise RuntimeError("Missing environment variable PROVIDER_API_KEY.")
        if not api_key.isascii():
            raise RuntimeError("PROVIDER_API_KEY contains non-ASCII characters.")

        payload = {
            "model": self._config.deepseek_model,
            "messages": messages,
            "stream": False,
        }
        data = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        request = urllib.request.Request(
            f"{self._config.deepseek_base_url}/chat/completions",
            data=data,
            method="POST",
            headers={
                "Content-Type": "application/json",
                "Authorization": f"Bearer {api_key}",
            },
        )

        with urllib.request.urlopen(request, timeout=self._config.request_timeout_seconds) as response:
            response_body = response.read().decode("utf-8")

        result = json.loads(response_body)
        choices = result.get("choices", [])
        if not choices:
            raise RuntimeError("DeepSeek returned no choices.")

        content = choices[0].get("message", {}).get("content", "").strip()
        if not content:
            raise RuntimeError("DeepSeek returned empty text.")
        return content

    def chat_json(self, messages: list[dict]) -> dict:
        return extract_json_object(self.chat(messages))
