#include <filesystem>

#include "settings.h"
#include "utils.h"
#include "command.h"
#include "latexwriter.h"

LatexWriter::LatexWriter() : mBaseName("report")
{
}

void LatexWriter::createPDFReport(const scheduler::scheduler &s) const
{
    recreate_Directory( getOutputPath_PDF() );
    createTex(s);
    runLatex();
}

void LatexWriter::runLatex() const
{
    timer tmr;
    loginfo("Running PDFLatex build to " + getOutputPath_PDF());

    std::string cmd = "cd "+getOutputPath_PDF()+" && pdflatex -interaction=nonstopmode "+mBaseName+".tex 2>&1";

    raymii::Command c;

    raymii::CommandResult r = c.exec(cmd);

    if (r.exitstatus != 0)
        throw(TerminateRunException(r.output));
    loginfo(S() << "PDFLatex Finished in " << tmr.stop() << " ms.");
}

std::string _title(unsigned int tablenum)
{
    switch (tablenum)
    {
        case 0:
            return "Customer Facing Defects and Bugs";
        case 1:
            return "Projects in Active Development";
        default:
            ;
    }
    return "Planned Projects";
}
bool _include(unsigned int tablenum, const scheduler::scheduledproject & z )
{
    static simpledate today;

    switch (tablenum)
    {
        case 0:
            return z.getType()==kPTBug;
        case 1:
            return z.mActualStart <= today;
        default:
            ;
    }
    return z.mActualStart > today;
}

void LatexWriter::createTex(const scheduler::scheduler &s) const
{
    simpledate today;

    std::string ofs_name = getOutputPath_PDF()+mBaseName+".tex";
    std::ofstream ofs(ofs_name);
    if (!ofs.is_open())
        TERMINATE("Unable to open file " + ofs_name + " for writing.");

    std::vector<scheduler::tProjectInfo> ProjectInfo;
    s.getProjectExtraInfo(ProjectInfo);


    ofs <<
R"END(
\documentclass[8pt]{extarticle}
\usepackage[a4paper,landscape,margin=12mm,top=18mm,bottom=18mm]{geometry}

\usepackage{tabularray}
\usepackage{xcolor}
\usepackage{needspace}

\begin{document}

{\Huge\textbf{\textsf{Active Development - )END" << today.getStr_nice_short() << R"END( }}}
\par $_{ }$ \par

This document lists projects approved for development and in the tech team scheduling system, from \textbf{)END";
ofs << gSettings().startDate().getStr_nice_long() << "}." << std::endl << std::endl;
ofs << "Total projects included: "<<s.getProjects().size()<<".\\\\"<<std::endl;
    

    for (unsigned int tablenum=0;tablenum<3;tablenum++)
    {
        int count=0;
        for (unsigned int i=0;i< s.getProjects().size();++i)
        {
            auto & z = s.getProjects().at(i);
            if (_include(tablenum,z))
            {
                if (count==0)
                    starttable(_title(tablenum),ofs);
                ++count;
                outputrow(count,z,ofs);
            }
        }
        if (count>0)
            endtable(ofs);
        loginfo(S()<<"Outputted "<<count<<" for "<<_title(tablenum));
    }

ofs <<
R"END(
\end{document}
)END";

    ofs.close();
}

void LatexWriter::outputrow(int n, const scheduler::scheduledproject &z, std::ofstream &ofs) const
{   
    bool late = false;
    if (!z.getTargetDate().isForever())
        if (z.mActualEnd > z.getTargetDate())
            late = true;


    ofs 
        << n
        << " & "
        << (z.getTargetDate().isForever() ? "" :  z.getTargetDate().getStr_nice_short())
        << " & "
        << (late ? "\\textcolor{red}{" : "")
        << z.mActualEnd.getStr_nice_short()
        << (late ? "}" : "")
        << " & "
        << z.getName()
        << " & "
        << z.getStatus()
        << " & "
        << z.getDesc()
        << " \\\\ " 
        << std::endl;
}

void LatexWriter::starttable(const std::string title, std::ofstream &ofs) const
{
ofs <<
R"END(

\Needspace{30\baselineskip}
\section{)END" << title << R"END(}

\begin{longtblr}[
  caption = {)END" << title << R"END(},
  label = {tab:proj},
]{
  colspec = {|p{0.025\linewidth} | p{0.08\linewidth} | p{0.08\linewidth} | p{0.2\linewidth} | p{0.2\linewidth} | p{0.275\linewidth}|},
  rowhead = 1,
  hlines,
  row{even} = {gray9},
  row{1} = {blue!30, font=\small\bfseries, c},
  column{1} = {r},
} 
\# & Business Target & Tech Complete & Project & Status/Notes & Description \\
)END";
}

void LatexWriter::endtable(std::ofstream &ofs) const
{
    ofs <<
R"END(
\end{longtblr}
)END";
}
