ngx.header.content_type = "text/html"

local prog = require'resty.exec'.new('/tmp/exec.sock')
prog.stdout = function(v)
    ngx.print(v)
    ngx.flush(true)
end

function readAll(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content:gsub("%s+", "")
end

-- prog('unbuffer','sudo','/var/www/scripts/update.sh')

ngx.say('<html><div style="white-space: pre; font-family: monospace;">')

ngx.say('Copying Jekyll Template...')
prog('mkdir','-p','/jpftemp/input')
prog('cp','-r','/example_data/template','/jpftemp')

ngx.say('<div style="color:grey;">')
ngx.say('Downloading Google Sheet to CSV input files...')
local spreadsheet=readAll('/config/jpf.spreadsheet')
ngx.say('Spreadsheet = '..spreadsheet)
prog('/root/.local/bin/gs-to-csv','--service-account-credential-file','/config/jpf.credentials.json',spreadsheet,'.*','/jpftemp/input')
ngx.say('</div>')

ngx.say('<hr/>')
ngx.say('Running jpf...')
prog('jpf','--html','/jpftemp')

ngx.say('Copying Output...')
prog('cp','-r','/jpftemp/output/html','/var/www')


ngx.say('<p>Done!</p>')
ngx.say('</div></html>')

-- prog('ls','/lua')
