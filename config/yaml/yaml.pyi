from typing import Any, Dict, List, TextIO, Union

def safe_load(stream: Union[str, bytes, TextIO]) -> Any: ...
def safe_load_all(stream: Union[str, bytes, TextIO]) -> Any: ...
def dump(data: Any, stream: Union[None, TextIO] = ..., **kwargs: Any) -> None: ...

class YAMLError(Exception): ...
