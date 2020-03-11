/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Martin Kelly, Anton Deguet
  Created on: 2011-02-15

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <map>
#include <sawCMUSphinx4/mtsCMUSphinx4.h>

typedef mtsCMUSphinx4::Context mtsCMUSphinx4Context;
CMN_IMPLEMENT_SERVICES(mtsCMUSphinx4Context);

#include <cisstCommon/cmnAssert.h>

mtsCMUSphinx4::Context::Context(const std::string & name):
    Name(name),
    OutOfGrammarFiltering(false)
{
    Words.SetOwner(*this);
}


bool mtsCMUSphinx4::Context::AddWordWithActions(const std::string & word, mtsCMUSphinx4::WordActions * wordActions)
{
    if (Words.AddItem(word, wordActions, CMN_LOG_LOD_INIT_ERROR)) {
        CMN_LOG_CLASS_INIT_DEBUG << "AddWord: added word \"" << word << "\""
                                 << " to context \"" << this->GetName() << "\"" << std::endl;
        return true;
    }
    CMN_LOG_CLASS_INIT_ERROR << "AddWord: failed to add word \"" << word << "\""
                             << " to context \"" << this->GetName() << "\"" << std::endl;
    return false;
}

bool mtsCMUSphinx4::Context::AddWord(const std::string & word)
{
    mtsCMUSphinx4::WordActions * wordActions = new mtsCMUSphinx4::WordActions();
    if (this->AddWordWithActions(word, wordActions)) {
        return true;
    }
    delete wordActions;
    return false;
}


bool mtsCMUSphinx4::Context::AddWordWithTransition(const std::string & word, const std::string & endContext)
{
    mtsCMUSphinx4::WordActions * wordActions = new mtsCMUSphinx4::WordActions(endContext);
    if (this->AddWordWithActions(word, wordActions)) {
        return true;
    }
    delete wordActions;
    return false;
}


const std::string & mtsCMUSphinx4::Context::GetName(void) const
{
	return this->Name;
}


std::vector<std::string> mtsCMUSphinx4::Context::GetVocabulary(void)
{
    std::vector<std::string> result;
    Words.GetNames(result);
	return result;
}


bool mtsCMUSphinx4::Context::PerformActionsForWord(const std::string & word)
{
    mtsCMUSphinx4::WordActions * wordActions = this->Words.GetItem(word);
    if (wordActions) {
        CMN_LOG_CLASS_RUN_DEBUG << "PerformActionsForWord: performing actions for word \"" << word << "\"" << std::endl;
        CMN_ASSERT(this->CMUSphinx4Wrapper);
        wordActions->PerformActions(this->CMUSphinx4Wrapper);
        return true;
    }
    CMN_LOG_CLASS_RUN_ERROR << "PerformActionsForWord: can't find word \"" << word
                            << "\" in context \"" << this->GetName() << "\"" << std::endl;
    return false;
}


bool mtsCMUSphinx4::Context::FilteringEnabled(void)
{
    return OutOfGrammarFiltering;
}


void mtsCMUSphinx4::Context::SetFiltering(bool filter)
{
    OutOfGrammarFiltering = filter;
}
