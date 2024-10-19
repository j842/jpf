#include <filesystem>

#include "settings.h"
#include "utils.h"
#include "command.h"
#include "latexwriter.h"

LatexWriter::LatexWriter()
{
}

void LatexWriter::createPDFReport(const scheduler::scheduler &s) const
{
}

void LatexWriter::runLatex()
{
    timer tmr;
    loginfo("Running PDFLatex build to " + getOutputPath_PDF());

    std::string cmd = "cd " + getOutputPath_Jekyll() + " ; pdflatex b 2>&1";

    raymii::Command c;

    raymii::CommandResult r = c.exec(cmd);

    if (r.exitstatus != 0)
        throw(TerminateRunException(r.output));
    loginfo(S() << "PDFLatex Finished in " << tmr.stop() << " ms.");
}

