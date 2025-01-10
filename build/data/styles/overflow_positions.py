import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))

from lang import Lang
from value import Value

overflow_positions = [
    Value().set_generate_name("safe").add_text(Lang.EN, "safe").add_text(Lang.FA, "امن"),
    Value().set_generate_name("unsafe").add_text(Lang.EN, "unsafe").add_text(Lang.FA, "نامناسب"),
]

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <overflow-position> = 
#   unsafe  |
#   safe    
