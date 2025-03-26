from fastapi import FastAPI, Request
from fastapi.responses import StreamingResponse, FileResponse
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
import subprocess
import json
import os
import time

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
app.mount("/static", StaticFiles(directory="/var/www/output/html"), name="static")

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

def stream_command_output(command):
    """Execute a command and stream its output in real-time."""
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
        universal_newlines=True
    )
    
    while True:
        output = process.stdout.readline()
        if output == '' and process.poll() is not None:
            break
        if output:
            yield f"data: {json.dumps({'output': output.strip()})}\n\n"
    
    return_code = process.poll()
    if return_code != 0:
        error = process.stderr.read()
        yield f"data: {json.dumps({'error': error})}\n\n"

@app.get("/update")
async def update(request: Request):
    """Serve the update.html page."""
    return FileResponse("/var/www/output/html/update.html")

@app.get("/update/stream")
async def update_stream(request: Request):
    async def event_generator():
        # Read spreadsheet ID
        spreadsheet = read_spreadsheet_id()
        if not spreadsheet:
            yield f"data: {json.dumps({'error': 'Could not read spreadsheet ID'})}\n\n"
            return

        # Execute all commands in sequence
        for step in COMMANDS:
            yield f"data: {json.dumps({'output': step['message']})}\n\n"
            
            for command in step['commands']:
                # Replace spreadsheet_id placeholder if present
                cmd = [arg.format(spreadsheet_id=spreadsheet) if '{spreadsheet_id}' in arg else arg 
                      for arg in command['cmd']]
                
                if command.get('type') == 'run':
                    try:
                        subprocess.run(cmd, check=True)
                    except subprocess.CalledProcessError as e:
                        yield f"data: {json.dumps({'error': f'Command failed: {e}'})}\n\n"
                        return
                else:  # default to stream
                    for output in stream_command_output(cmd):
                        yield output

        yield f"data: {json.dumps({'output': 'SUCCESS: Website has been regenerated successfully!'})}\n\n"

    return StreamingResponse(
        event_generator(),
        media_type="text/event-stream"
    )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000) 