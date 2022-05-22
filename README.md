# jpf

John's Project Forecaster

- Auto schedules *REMAINING* work for projects, from date specified in settings.csv
- Does nothing for historical work or review.
- Constraints are:
   - resources (people), and can have multiple on a task (even workload assumed),
   - number of dev days per task total (at full capacity),
   - minimum calendar days to complete a task,
   - multiple cross-team dependencies on other tasks,
   - available capacity per person (max availability specified in teams.csv, based on BAU workload),
   - specified holiday periods

## Installing Binary from Debian/Ubuntu Package

Intended to work on Debian 11 (Bullseye), Ubuntu 20.04 (Focal) and Ubuntu 22.04 (Jammy). Does not work on Debian 10 (Buster) or Ubuntu 18.04 (Bionic) as g++ and libs are too old.

```
sudo apt update
sudo apt install -y curl gnupg apt-transport-https
```

Installing the key seems to require being in a root shell (not sudo):
```
sudo su -
curl -fsSL https://packagecloud.io/j842/main/gpgkey | gpg --dearmor > /usr/share/keyrings/j842_main-archive-keyring.gpg
exit
```

Then create `/etc/apt/sources.list.d/j842_main.list` with content:
```
deb [signed-by=/usr/share/keyrings/j842_main-archive-keyring.gpg] https://packagecloud.io/j842/main/any/ any main
deb-src [signed-by=/usr/share/keyrings/j842_main-archive-keyring.gpg] https://packagecloud.io/j842/main/any/ any main
```
Then `sudo apt update` again to get the new repo info.

You should now be able to install and update using apt as you wish.  
  
Install:  
`sudo apt install jpf`  
  
Upgrade:  
`sudo apt install --only-upgrade jpf`



## Running jpf

### Manually

`jpf .`

Files are written to the output/ folder.

### Watch for changes

In this mode jpm starts a webserver (webfsd) on port 5000, displaying the HTML output of jpf. 
It then watches the input folder and if any changes are made reschedules and recreates all outputs.
The pages automatically update in most web browsers (you shouldn't need to refresh the page).

```
jpf -watch .
```

Connect to `http://localhost:5000`

## Related Software

A good CSV editor is Modern CSV:  https://www.moderncsv.com/
Crossplatform and makes it easy to move tasks around.

The Gantt chart software to read the output Gantt.csv is GanttProject:
https://www.ganttproject.biz/



## Development Setup (Compiling jpf from source)

On either Ubuntu 20.04 or Debian 11 (including via WSL2 under Windows):
```
sudo apt install build-essential libboost-date-time-dev libcppunit-dev podman gh ruby-full zlib1g-dev
sudo gem install jekyll bundler webrick
```

Clone:
```
git clone https://github.com/j842/jpf.git
cd jpf
```

Set up relevant folders and build the podman images:
```
make setup
make images
```

Then compile jpf itself:
```
make
```
And test:

```
./jpf -t
./jpf .
./jpf -w .
```

## Make the jpf debian package
```
make deb
```

To upload, make sure ~/.packagecloud exsts with the correct token:
```
{"url":"https://packagecloud.io", "token": "packagecloud_token"}
```
(Note this token is baked into the podman image, so do not make that image public)
```
make upload
```

### Make the webfsd-jpf debian package

```
make -C deps/webfsd-jpf
make -C deps/webfsd-jpf upload
```


## Project Tips

- Put tasks assigned to a person in the team the person is in, not the team doing most of the project work.
- The order of items in the team list is the items priority. This should be in line with the overall project priorities (but is not enforced).
- Team item lists should be updated on the same day and the date updated in settings.csv - e.g. once per week, or once per sprint.



## Visual Studio Code C++
```
Ctrl + Shift + P then select C/C++:Edit Configurations (JSON)

Adjust the content for cStandard and cppStandard:
   "compilerPath": "/usr/bin/gcc-9",
   "cStandard": "gnu17",
   "cppStandard": "gnu++17",
```

## WSL-2 memory
Increasing memory to 16Gb was preiously needed, but is not essential now:   
https://clay-atlas.com/us/blog/2021/08/31/windows-en-wsl-2-memory/


## Github Actions

This repo contains two github actions:

- test_jpf, which runs the unit tests on any commit to Main
- deploy_package, which is manually invoked. It builds the debian package and uploads it to the official repository. 
