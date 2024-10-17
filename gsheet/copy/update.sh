#!/bin/bash

function fatal()
{
    msg=$@

    echo "<div style=\"color:red;\">"
    echo "${msg}"
    echo "</div>"
    exit -1
}

flush() {
    padding=4100
    dd if=/dev/zero bs=$padding count=1 2>/dev/null
}

ACCTFILE="/config/jpf.credentials.json"
SPREADSHEET=$(</config/jpf.spreadsheet)
TEMPDIR="/jpftemp"
INPUTDIR="$TEMPDIR/input"

cat << ~~~
HTTP/1.1 200 OK
Content-Type: text/html
Cache-Control: no-cache
X-Accel-Buffering: no
<html>
<div style="white-space: pre; font-family: monospace;">

~~~

USR="$(whoami)"

echo "Temp Directory: [${TEMPDIR}]"
echo "Spreadsheet Id: [${SPREADSHEET}]"
echo "Running as:     [${USR}]"
echo " "


if [ "$(id -u)" -ne 0 ]; then
        echo 'This script must be run by root' >&2
        exit 1
fi


#sudo /usr/bin/update_jpf.sh

[[ ! -e ${TEMPDIR} ]] || rm -rf ${TEMPDIR}
mkdir -p ${TEMPDIR}
mkdir -p ${INPUTDIR}

cp -r /example_data/template ${TEMPDIR}

echo "<div style=\"color:grey;\">"
echo "Downloading Google Sheet to CSV input files..."

flush

/root/.local/bin/gs-to-csv --service-account-credential-file "${ACCTFILE}" "${SPREADSHEET}" '.*' "${INPUTDIR}"
echo "</div>"
echo "<div>"
echo "Running JPF..."

flush

jpf --html "${TEMPDIR}" 2>&1

if [ $? -ne 0 ]; then
    echo "</div>"
    fatal "jpf compilation failed. Update script aborted."
fi

echo "</div>"

echo "<div>"
echo "Copying folders..."

flush

rm -rf "/var/www/html/*"
cp -r "${TEMPDIR}/output/html" "/var/www"

echo ""
echo "</div><div style=\"color:darkgreen;\">"
echo "All done!"
echo "</div>"

echo "</div>"
echo "</html>"

flush