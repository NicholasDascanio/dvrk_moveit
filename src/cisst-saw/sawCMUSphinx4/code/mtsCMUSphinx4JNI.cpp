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

#include <string>
#include "mtsCMUSphinx4JNI.h"
#include <sawCMUSphinx4/mtsCMUSphinx4.h>

void mtsCMUSphinx4JavaWordRecognizedCallback(mtsCMUSphinx4 * sphinx4Wrapper,
                                          const std::string & word)
{
    sphinx4Wrapper->WordRecognizedCallback(word);
}


void JNICALL Java_sawCMUSphinx4_WordRecognizedCallback(JNIEnv *env, jobject, jlong sphinx4WrapperPointer, jstring wordJava)
{
    // convert JString to std::string
    const char * wordCharPointer = env->GetStringUTFChars(wordJava, 0);
    const std::string word(wordCharPointer);
    mtsCMUSphinx4 * sphinx4Wrapper = reinterpret_cast<mtsCMUSphinx4 *>(sphinx4WrapperPointer);
    CMN_ASSERT(sphinx4Wrapper != 0);
    mtsCMUSphinx4JavaWordRecognizedCallback(sphinx4Wrapper, word);
    // release Java string
    env->ReleaseStringUTFChars(wordJava, wordCharPointer);
}
