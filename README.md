# jpf

John's Project Forecaster

- Auto schedules *REMAINING* work for projects, from date specified in settings.csv
- Does nothing for historical work or review - this data is not kept in any way.
- Constraints are:
   - resources (people) availability; can have multiple on a task (will finish as quickly as it can).
   - blocking resources will prevent a task starting until they are available.
   - contributing resources will help if they are free, but are not essential to progress the task.
   - minimum calendar days to complete a task.
   - any cross-team dependencies on other tasks.
   - available capacity per person (max availability specified in teams.csv, based on BAU workload).
   - specified holiday periods.


## Running jpf

- Copy the example data to wherever you want to store your project data.
- Run jpf on this directory.

## Related Software

A simple CSV editor is Modern CSV:  https://www.moderncsv.com/
Crossplatform and makes it easy to move tasks around. Visual Studio
Code has a number of plugins to help with CSV also.

The Gantt chart software to read the output Gantt.csv is GanttProject:
https://www.ganttproject.biz/


## Your own graphs/themes

You can modify the Jekyll templates to create your own look and feel for the HTML files generated, or to add new graphs or representations of the scheduled work.

In the `website` folder, all the Jekyll (liquid) files are in `template`. There is example data in `input`, and you can 
build the website using Jekyll with the `build.sh` script.                                                  


## Development Setup (Compiling jpf from source)

On Ubuntu 24.04:
```
sudo apt install build-essential libboost-date-time-dev libcppunit-dev podman gh ruby-full zlib1g-dev texlive-latex-base texlive-latex-extra

echo '# Install Ruby Gems to ~/gems' >> ~/.bashrc
echo 'export GEM_HOME="$HOME/gems"' >> ~/.bashrc
echo 'export PATH="$HOME/gems/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
MAKE="make -j $(nproc)" gem install jekyll bundler --no-document
```

Clone:
```
git clone https://github.com/j842/jpf.git
cd jpf
```

Then compile jpf itself and test it:
```
make
make check
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

This repo contains four github actions in `.github/workflows`, which should be self explanatory.
