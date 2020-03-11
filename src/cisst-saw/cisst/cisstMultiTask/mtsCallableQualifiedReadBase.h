/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s): Anton Deguet
  Created on: 2010-10-05

  (C) Copyright 2010 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


/*!
  \file
  \brief Defines a base class for a callable object with a qualifer and a placeholder to retrieve data
 */

#ifndef _mtsCallableQualifiedReadBase_h
#define _mtsCallableQualifiedReadBase_h

#include <cisstMultiTask/mtsExecutionResult.h>

/*!
  \ingroup cisstMultiTask

  A base class command object with an execute method that takes a
  qualifier and uses a placeholder to retrieve data.  This pure
  virtual base class is derived to support either global functions or
  methods with the signature "void method(const _argumentType &
  argument, _resultType & result) const" */
class mtsCallableQualifiedReadBase
{
public:

    /*! The constructor. Does nothing */
    inline mtsCallableQualifiedReadBase(void) {}

    /*! The destructor. Does nothing */
    inline virtual ~mtsCallableQualifiedReadBase() {}

    /*! The execute method. Abstract method to be implemented by derived
      classes to run the actual operation on the receiver. */
    virtual mtsExecutionResult Execute(const mtsGenericObject & argument,
                                       mtsGenericObject & result) = 0;

    /*! Human readable description */
    virtual void ToStream(std::ostream & outputStream) const = 0;
};

/*! Stream out operator for all classes derived from mtsCallableQualifiedReadBase.
  This operator uses the ToStream method so that the output can be
  different for each derived class. */
inline std::ostream & operator << (std::ostream & outputStream,
                                   const mtsCallableQualifiedReadBase & callable) {
    callable.ToStream(outputStream);
    return outputStream;
}

#endif // _mtsCallableQualifiedReadBase_h
