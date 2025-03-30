from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import FileResponse
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
import subprocess
import json
import os
import time
import asyncio
import sys
from starlette.responses import Response

app = FastAPI()

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Mount static files
#app.mount("/static", StaticFiles(directory="/website/static"), name="static")
#app.mount("/output", StaticFiles(directory="/var/www/output/html"), name="output")

# Define all commands to be run
COMMANDS = [
    {
        "message": "Setting up working directory...",
        "commands": [
            {"cmd": ["mkdir", "-p", "/jpftemp/input"], "type": "run"},
            {"cmd": ["cp", "-r", "/website/template", "/jpftemp"], "type": "run"}
        ]
    },
    {
        "message": "Downloading Google Sheet to CSV input files...",
        "commands": [
            {
                "cmd": ["/root/.local/bin/gs-to-csv", "-f", "--service-account-credential-file", 
                       "/config/jpf.credentials.json", "{spreadsheet_id}", ".*", "/jpftemp/input"]
            }
        ]
    },
    {
        "message": "Running JPF to generate website...",
        "commands": [
            {"cmd": ["jpf", "--html", "/jpftemp"]}
        ]
    },
    {
        "message": "Copying generated website to web root...",
        "commands": [
            {"cmd": ["cp", "-r", "/jpftemp/output", "/var/www"], "type": "run"}
        ]
    }
]

def read_spreadsheet_id():
    """Read the Google Sheets ID from the configuration file."""
    try:
        with open('/config/jpf.spreadsheet', 'r') as f:
            return f.read().strip()
    except Exception as e:
        return None

async def send_message(websocket: WebSocket, message: str, color: str, close: bool = False):
    """Send a message to the client via WebSocket."""
    data = {
        'output': message,
        'color': color
    }
    if close:
        data['close'] = True
    await websocket.send_json(data)
    # Remove the empty text message that's causing JSON parse errors
    # await websocket.send_text("")

async def stream_command_output(command, websocket: WebSocket):
    """Execute a command and stream its output in real-time."""
    process = await asyncio.create_subprocess_exec(
        *command,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE,
        env={**os.environ, 'PYTHONUNBUFFERED': '1'}
    )

    async def read_stream(stream, color):
        while True:
            line = await stream.readline()
            if not line:
                break
            line = line.decode().strip()
            if line:
                await send_message(websocket, line, color)

    # Read stdout and stderr concurrently
    await asyncio.gather(
        read_stream(process.stdout, 'black'),
        read_stream(process.stderr, 'red')
    )

    # Wait for the process to complete
    return_code = await process.wait()
    if return_code != 0:
        await send_message(websocket, f"Command failed with return code {return_code}", 'red')

@app.get("/update")
async def update():
    """Serve the update.html page with no-cache headers."""
    response = FileResponse("/website/static/update.html")
    response.headers["Cache-Control"] = "no-store, no-cache, must-revalidate, max-age=0"
    response.headers["Pragma"] = "no-cache"
    response.headers["Expires"] = "0"
    return response

@app.websocket("/update/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    
    try:
        # Read spreadsheet ID
        spreadsheet = read_spreadsheet_id()
        if not spreadsheet:
            await send_message(websocket, 'Could not read spreadsheet ID', 'red')
            await send_message(websocket, '<br>Connection closed due to error.', 'red', close=True)
            return

        # Execute all commands in sequence
        for step in COMMANDS:
            await send_message(websocket, step['message'], 'blue')
            
            for command in step['commands']:
                # Replace spreadsheet_id placeholder if present
                cmd = [arg.format(spreadsheet_id=spreadsheet) if '{spreadsheet_id}' in arg else arg 
                      for arg in command['cmd']]
                
                if command.get('type') == 'run':
                    try:
                        process = await asyncio.create_subprocess_exec(
                            *cmd,
                            stdout=asyncio.subprocess.PIPE,
                            stderr=asyncio.subprocess.PIPE
                        )
                        await process.wait()
                        if process.returncode != 0:
                            raise subprocess.CalledProcessError(process.returncode, cmd)
                    except subprocess.CalledProcessError as e:
                        await send_message(websocket, f'Command failed: {e}', 'red')
                        await send_message(websocket, '<br>Connection closed due to error.', 'red', close=True)
                        return
                else:  # default to stream
                    await stream_command_output(cmd, websocket)

        await send_message(websocket, 'SUCCESS: Website has been regenerated successfully!', 'green')
        await send_message(websocket, '<br>All done.', 'green', close=True)

    except WebSocketDisconnect:
        print("Client disconnected")
    except Exception as e:
        print(f"Error: {e}")
        try:
            await send_message(websocket, f'Error: {str(e)}', 'red')
            await send_message(websocket, '<br>Connection closed due to error.', 'red', close=True)
        except:
            pass

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000) 