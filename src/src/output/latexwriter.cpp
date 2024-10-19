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

void LatexWriter::createTex(const scheduler::scheduler &s) const
{
    std::string ofs_name = getOutputPath_PDF()+mBaseName+".tex";
    std::ofstream ofs(ofs_name);
    if (!ofs.is_open())
        TERMINATE("Unable to open file " + ofs_name + " for writing.");

    ofs <<
R"END(
\documentclass[8pt]{extarticle}
\usepackage[a4paper,margin=20mm,left=12mm,right=12mm]{geometry}

\usepackage{tabularray}
\usepackage{xcolor}

\begin{document}

Projects approved for development, as of )END";
ofs << gSettings().startDate().getStr_nice_long() << "." << std::endl;

    std::vector<scheduler::tProjectInfo> ProjectInfo;
    s.getProjectExtraInfo(ProjectInfo);

    int ap=0;
    for (unsigned int i=0;i< s.getProjects().size();++i)
    {
        auto & z = s.getProjects().at(i);
        if (z.mActualStart.getGregorian() <= simpledate().getGregorian())
        {
            if (ap==0)
                starttable("Development Started",ofs);
            outputrow(z,ofs);
            ++ap;
        }
    }
    if (ap>0)
        endtable(ofs);
    loginfo(S()<<"Outputted "<<ap<<" active projects.");
    

    int sp=0;
    for (unsigned int i=0;i< s.getProjects().size();++i)
    {
        auto & z = s.getProjects().at(i);
        if (z.mActualStart.getGregorian() > simpledate().getGregorian())
        {
            if (sp==0)
                starttable("Scheduled Projects",ofs);
            outputrow(z,ofs);
            ++sp;
        }
    }
    if (sp>0)
        endtable(ofs);
    loginfo(S()<<"Outputted "<<sp<<" scheduled projects.");

ofs <<
R"END(
\end{document}
)END";

    ofs.close();
}

void LatexWriter::outputrow(const scheduler::scheduledproject &z, std::ofstream &ofs) const
{
    ofs 
        << simpledate(z.mActualEnd.getGregorian()+ boost::gregorian::days(7)).getStr_nice_short() 
        << " & "
        << z.mActualEnd.getStr_nice_short()
        << " & "
        << z.getName()
        << " & "
        << z.getmComments()
        << " & "
        << z.getDesc()
        << " \\\\ " 
        << std::endl;
}

void LatexWriter::starttable(const std::string title, std::ofstream &ofs) const
{
ofs <<
R"END(

\section{)END" << title << R"END(}

\begin{longtblr}[
  caption = {)END" << title << R"END(},
  label = {tab:proj},
]{
  colspec = {|p{0.1\linewidth} | p{0.1\linewidth} | p{0.2\linewidth} | p{0.16\linewidth} | p{0.3\linewidth}|},
  rowhead = 1,
  hlines,
  row{even} = {gray9},
  row{1} = {blue!30, font=\small\bfseries, c},
} 
Release & Code Complete & Project & Status/Notes & Description \\
)END";
}

void LatexWriter::endtable(std::ofstream &ofs) const
{
    ofs <<
R"END(
\end{longtblr}
)END";
}
