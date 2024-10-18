ngx.header.content_type = "text/html"

local prog = require'resty.exec'.new('/tmp/exec.sock')
prog.stdout = function(v)
    ngx.print(v)
    ngx.flush(true)
end
prog.stderr = function(v)
    ngx.print(v)
    ngx.flush(true)
end

function readAll(file)
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
prog('mkdir','-p','/jpftemp/input')
prog('cp','-r','/website/template','/jpftemp')

-------------------------------------------------------------------------------

ngx.say('<hr/>')
ngx.say('<div style="color:grey;">')
ngx.say('Downloading Google Sheet to CSV input files...')
ngx.flush(true)
local spreadsheet=readAll('/config/jpf.spreadsheet')
ngx.say('Spreadsheet = '..spreadsheet)
ngx.flush(true)
prog('/root/.local/bin/gs-to-csv','-f','--service-account-credential-file','/config/jpf.credentials.json',spreadsheet,'.*','/jpftemp/input')
ngx.say('</div>')

-------------------------------------------------------------------------------

ngx.say('<hr/>')
ngx.say('Running jpf...')
ngx.flush(true)
prog('jpf','--html','/jpftemp')

-------------------------------------------------------------------------------

ngx.say('<hr/>')
ngx.say('Copying Output...')
ngx.flush(true)
prog('cp','-r','/jpftemp/output/html','/var/www')

-------------------------------------------------------------------------------

ngx.say('<p>Done!</p>')
ngx.say('</div></html>')
ngx.flush(true)

-- prog('ls','/lua')
