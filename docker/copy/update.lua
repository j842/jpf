-- Set command timeout (in seconds)
local CMD_TIMEOUT = 120  -- 2 minutes per command

-- Detect if we're running in Nginx context
local is_nginx = ngx ~= nil

-- Function to safely print that works in both contexts
local function safe_print(str)
    if str then
        if is_nginx then
            ngx.print(str)
            ngx.flush(true)
        else
            print(str)
            io.flush()
        end
    end
end

-- Function to output HTML if in Nginx context, plain text otherwise
local function output_message(str, is_error)
    if is_nginx then
        if is_error then
            safe_print('<span style="color: red;">' .. str .. '</span>')
        else
            safe_print(str)
        end
    else
        if is_error then
            safe_print('ERROR: ' .. str)
        else
            safe_print(str)
        end
    end
end

local prog = require'resty.exec'.new('/tmp/exec.sock')

prog.stdout = function(v)
    safe_print(v)
end

prog.stderr = function(v)
    output_message(v, true)
end

local function run_command(...)
    local args = {...}
    safe_print('Running command: ' .. table.concat(args, ' ') .. '\n')
    
    -- Set timeout for the command
    prog.timeout = CMD_TIMEOUT * 1000  -- Convert to milliseconds
    
    local res, err = prog(...)
    if err then
        if err:match("timeout") then
            output_message('Command timed out after ' .. CMD_TIMEOUT .. ' seconds\n', true)
        else
            output_message(err .. '\n', true)
        end
        return false
    end
    return true
end

local function readAll(file)
    local f = io.open(file, "rb")
    if not f then return nil, "Could not open file: " .. file end
    local content = f:read("*all")
    f:close()
    return content and content:gsub("%s+", "") or nil, not content and "File is empty: " .. file or nil
end

-- Initialize output based on context
if is_nginx then
    ngx.header.content_type = "text/html"
    safe_print('<html><div style="white-space: pre; font-family: monospace;">')
end

-- Read spreadsheet ID at the start
local spreadsheet, err = readAll('/config/jpf.spreadsheet')
if not spreadsheet then
    output_message(err, true)
    if is_nginx then
        safe_print('</div></html>')
    end
    return
end

output_message('Starting update process...', false)
output_message('----------------------------------------', false)

-------------------------------------------------------------------------------

output_message('Copying Jekyll Template...', false)
if not run_command('mkdir','-p','/jpftemp/input') then goto error end
if not run_command('cp','-r','/website/template','/jpftemp') then goto error end

-------------------------------------------------------------------------------

output_message('----------------------------------------', false)
output_message('Downloading Google Sheet to CSV input files...', false)
output_message('Spreadsheet = '..spreadsheet, false)
if not run_command('/root/.local/bin/gs-to-csv','-f','--service-account-credential-file','/config/jpf.credentials.json',spreadsheet,'.*','/jpftemp/input') then goto error end

-------------------------------------------------------------------------------

output_message('----------------------------------------', false)
output_message('Running jpf...', false)
if not run_command('jpf','--html','/jpftemp') then goto error end

-------------------------------------------------------------------------------

output_message('----------------------------------------', false)
output_message('Copying Output...', false)
if not run_command('cp','-r','/jpftemp/output','/var/www') then goto error end

-------------------------------------------------------------------------------

output_message('----------------------------------------', false)
output_message('SUCCESS: Process completed successfully!', false)
if is_nginx then
    safe_print('</div></html>')
end
do return end

::error::
output_message('----------------------------------------', false)
output_message('ERROR: Process failed. Check the error messages above.', true)
if is_nginx then
    safe_print('</div></html>')
end

-- prog('ls','/lua')
