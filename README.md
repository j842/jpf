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

## Setup

On either Ubuntu 20.04 or WSL2 under Windows:
```
sudo apt-get install libboost-date-time-dev build-essential webfs ruby ruby-dev 

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install gcc-9 g++-9

sudo gem install fpm
sudo gem install rake rainbow package_cloud
git clone https://github.com/j842/jpf.git
cd jpf
make
```
Notes:
- It is possbile on Ubuntu 18.04 if G++-9 is installed and configured as here: https://linuxconfig.org/how-to-switch-between-multiple-gcc-and-g-compiler-versions-on-ubuntu-20-04-lts-focal-fossa 
- Ruby 3 doesn't work with package_cloud :( 

## Make and upload debian package
```
make deb
```

## User install

```
sudo apt-get update
sudo apt-get install -y curl gnupg apt-transport-https
```

Installing the key seems to require being in a root shell (not sudo):
```
sudo su -
curl -fsSL https://packagecloud.io/j842/main/gpgkey | gpg --dearmor > /usr/share/keyrings/j842_main-archive-keyring.gpg
sudo nano -w /etc/apt/sources.list.d/j842_main.list
```
And add the content
```
deb [signed-by=/usr/share/keyrings/j842_main-archive-keyring.gpg] https://packagecloud.io/j842/main/any/ any main
deb-src [signed-by=/usr/share/keyrings/j842_main-archive-keyring.gpg] https://packagecloud.io/j842/main/any/ any main
```
Then `exit`, and `sudo apt-get update` again to get the new repo info.

You should now be able to install and update using apt as you wish.  
Install: `sudo apt-get install jpf`  
Upgrade: `sudo apt-get upgrade jpf`



## Running

### Manually

`./jpf`

Files are written to the output/ folder.

### Watch for changes

In this mode jpm starts a webserver (webfsd) on port 5000, displaying `output/html/index.html`. 
It then watches the `input` folder and if any changes are made reschuleds and recreates all outputs.
index.html then automatically updates in most web browsers (you shouldn't need to refresh the page).

```
./jpf watch
```

Connect to `http://localhost:5000` - which works now in WSL2 as well as Ubuntu.

If you have problems accessing the webpage with WSL2, make sure you've correctly updated:  
https://docs.microsoft.com/en-us/windows/wsl/install-manual
  
and check your instance is on version 2 from PowerShell with  
`wsl -l -v`

## Tips

- Put tasks assigned to a person in the team the person is in, not the team doing most of the project work.
- The order of items in the team list is the items priority. This should be in line with the overall project priorities (but is not enforced).
- Team item lists should be updated on the same day and the date updated in settings.csv - e.g. once per week, or once per sprint.

A good CSV editor is Modern CSV:  https://www.moderncsv.com/
Crossplatform and makes it easy to move tasks around.

The Gantt chart software to read the output Gantt.csv is GanttProject:
https://www.ganttproject.biz/


## Visual Studio Code C++
```
Ctrl + Shift + P then select C/C++:Edit Configurations (JSON)

Adjust the content for cStandard and cppStandard:
   "compilerPath": "/usr/bin/gcc-9",
   "cStandard": "gnu17",
   "cppStandard": "gnu++17",
```
