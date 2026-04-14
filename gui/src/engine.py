import subprocess

from state import current_fen

# INITIAL FEN: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"]

def call_engine(fen: str = None) -> str:
    """
    @brief Invokes the C++ chess engine executable with an optional FEN string.

    Launches the compiled engine binary as a subprocess, optionally passing a
    FEN string as a command-line argument. Captures and returns the engine's
    standard output.

    @param fen Optional FEN string representing the board state
    @return Output string produced by the engine (stdout)

    @raises RuntimeError if the engine returns a non-zero exit code

    @note
    - Assumes executable is located at './engine/chess'
    - Uses subprocess without shell for safety
    - FEN string is passed as a single argument (handles spaces correctly)

    @complexity O(1) Python-side (external process dominates runtime)
    """
    command = ['./engine/chess']

    #If no argument passed, sets initial position of board
    if isinstance(fen, str):
        command.append(fen)

    result = subprocess.run(command, capture_output=True, text=True)

    # Handle errors explicitly
    if result.returncode != 0:
        raise RuntimeError(f"Engine error:\n{result.stderr.strip()}")

    return result.stdout.strip()

print(call_engine(current_fen))