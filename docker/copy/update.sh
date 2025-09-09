#!/bin/bash

# Set content type for HTML output
echo "Content-Type: text/html"
echo

# Start HTML output
echo '<html><div style="white-space: pre; font-family: monospace;">'

# Function to output messages
output_message() {
    echo "$1"
    echo
    # Flush output
    exec 1>&1
}

# Function to output error messages
output_error() {
    echo "<span style='color: red;'>$1</span>"
    echo
    exec 1>&1
}

# Read spreadsheet ID
if [ ! -f /config/jpf.spreadsheet ]; then
    output_error "Could not open file: /config/jpf.spreadsheet"
    echo '</div></html>'
    exit 1
fi

SPREADSHEET=$(cat /config/jpf.spreadsheet | tr -d '[:space:]')
if [ -z "$SPREADSHEET" ]; then
    output_error "File is empty: /config/jpf.spreadsheet"
    echo '</div></html>'
    exit 1
fi

# Execute update process
output_message "Starting update process..."
output_message "----------------------------------------"

# Copy Jekyll Template
output_message "Copying Jekyll Template..."
mkdir -p /jpftemp/input
if ! cp -r /website/template /jpftemp; then
    output_error "Failed to copy template"
    echo '</div></html>'
    exit 1
fi

output_message "----------------------------------------"

# Download Google Sheet
output_message "Downloading Google Sheet to CSV input files..."
output_message "Spreadsheet = $SPREADSHEET"
if ! /root/.local/bin/gs-to-csv -f --service-account-credential-file /config/jpf.credentials.json "$SPREADSHEET" ".*" /jpftemp/input; then
    output_error "Failed to download Google Sheet"
    echo '</div></html>'
    exit 1
fi

output_message "----------------------------------------"

# Run jpf
output_message "Running jpf..."
if ! jpf --html /jpftemp; then
    output_error "Failed to run jpf"
    echo '</div></html>'
    exit 1
fi

output_message "----------------------------------------"

# Copy Output
output_message "Copying Output..."
if ! cp -r /jpftemp/output /var/www; then
    output_error "Failed to copy output"
    echo '</div></html>'
    exit 1
fi

output_message "----------------------------------------"
output_message "SUCCESS: Process completed successfully!"
echo '</div></html>' 