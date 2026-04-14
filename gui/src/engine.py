import subprocess

from state import current_fen

# INITIAL FEN: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
def call_engine(fen:str):
    output = subprocess.run(['./engine/chess', fen], capture_output=True, text=True)
    return output.stdout

print(call_engine(current_fen))