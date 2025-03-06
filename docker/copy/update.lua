-- Set headers for streaming response
ngx.header.content_type = "text/html"

local prog = require'resty.exec'.new('/tmp/exec.sock')

-- Function to safely print and flush with invisible padding
local function safe_print(str)
    if str then
        ngx.print(str .. '<!--' .. string.rep(' ', 1024) .. '-->') -- Invisible padding
        ngx.flush(true)
    end
end

prog.stdout = function(v)
    safe_print(v)
end

prog.stderr = function(v)
    safe_print('<span style="color: red;">' .. v .. '</span>')
end

local function run_command(...)
    local res, err = prog(...)
    if err then
        safe_print('<span style="color: red;">Error: ' .. err .. '</span>')
    end
end

local function readAll(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content:gsub("%s+", "")
end

ngx.say('<html><div style="white-space: pre; font-family: monospace;">')

-------------------------------------------------------------------------------

ngx.say('<hr/>')
ngx.say('Copying Jekyll Template...')
ngx.flush(true)
run_command('mkdir','-p','/jpftemp/input')
run_command('cp','-r','/website/template','/jpftemp')

-------------------------------------------------------------------------------

ngx.say('<hr/>')
ngx.say('<div style="color:grey;">')
ngx.say('Downloading Google Sheet to CSV input files...')
ngx.flush(true)
local spreadsheet=readAll('/config/jpf.spreadsheet')
ngx.say('Spreadsheet = '..spreadsheet)
ngx.flush(true)
run_command('/root/.local/bin/gs-to-csv','-f','--service-account-credential-file','/config/jpf.credentials.json',spreadsheet,'.*','/jpftemp/input')
ngx.say('</div>')

-------------------------------------------------------------------------------

ngx.say('<hr/>')
ngx.say('Running jpf...')
ngx.flush(true)
run_command('jpf','--html','/jpftemp')

-------------------------------------------------------------------------------

ngx.say('<hr/>')
ngx.say('Copying Output...')
ngx.flush(true)
run_command('cp','-r','/jpftemp/output','/var/www')

-------------------------------------------------------------------------------

ngx.say('<p>Done!</p>')
ngx.say('</div></html>')
ngx.flush(true)

-- prog('ls','/lua')
