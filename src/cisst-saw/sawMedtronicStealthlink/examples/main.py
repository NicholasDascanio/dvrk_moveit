#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Mon Aug 29 12:27:27 2011

@author: wen
"""

# Python standard library
import ctypes
import platform
import sys

# cisst
if platform.system() == 'Linux':
    sys.setdlopenflags(sys.getdlopenflags() | ctypes.RTLD_GLOBAL)
import time
import os
import cisstMultiTaskPython as mts
import cisstParameterTypesPython as prm
import cisstCommonPython as cmn


cmn.cmnLogger_SetMask(cmn.CMN_LOG_ALLOW_ALL)
cmn.cmnLogger_SetMaskFunction(cmn.CMN_LOG_ALLOW_ALL)
cmn.cmnLogger_SetMaskDefaultLog(cmn.CMN_LOG_ALLOW_ALL)
cmn.cmnLogger_SetMaskClassMatching('mts', cmn.CMN_LOG_ALLOW_ALL)

print 'starting sawMedtronicStealthlinkClientComponent.py...'
manager = mts.mtsManagerLocal.GetInstance()
stealth = mts.mtsComponentWithManagement('stealthProxy')
manager.AddComponent(stealth)
print 'added stealthProxy Component to local manager'
manager.CreateAll()
manager.StartAll()
print 'starting sawMedtronicStealthlinkClientComponent.py...'
services = stealth.GetManagerComponentServices()
valid = services.Load('libsawMedtronicStealthlink.so')
if(bool(valid)):
    print 'loaded libsawMedtronicStealthlink.so'
else:
    print 'ERROR services unable to load libsawMedtronicStealthlink.so'
    ctypes.CDLL('libsawMedtronicStealthlink.so')
    print 'CDLL loaded libsawMedtronicStealthlink.so'
ctorArg = mts.mtsTaskPeriodicConstructorArg("Stealthlink", 50.0)
print 'created ctorArg'
valid = services.ComponentCreate('mtsMedtronicStealthlink',ctorArg)
if(bool(valid)):
    print 'Created stealthlink component!!!'
else:
    print 'ERROR Could not create stealthlink component'

stealthComponent = manager.GetComponent('Stealthlink')

stealthComponent.Configure('config.xml')
print 'configured stealthlink component'

stealth.AddInterfaceRequiredAndConnect(('Stealthlink', 'Pointer'))
stealth.AddInterfaceRequiredAndConnect(('Stealthlink', 'Frame'))
stealth.AddInterfaceRequiredAndConnect(('Stealthlink', 'Registration'))

collector = mts.mtsCollectorState(stealthComponent.GetName(),'StateTable',mts.mtsCollectorBase.COLLECTOR_FILE_FORMAT_CSV)
valid = collector.AddSignal('ToolData')
if(bool(valid)):
    print 'collector signals added'
else:
    print 'Error collector signals'
valid = manager.AddComponent(collector)
if(bool(valid)):
    print 'collector component added to manager'
else:
    print 'Error adding collector component'
valid = collector.Connect()
if(bool(valid)):
    print 'collector connected'
else:
    print 'Error collector not connected'
    
time.sleep(2.0)
    
manager.CreateAll()
print 'waiting after create all...'
time.sleep(1.0)
#manager.WaitForStateAll(mts.mtsComponentState(mts.mtsComponentState.READY))
manager.StartAll()
print 'waiting after start all...'
#manager.WaitForStateAll(mts.mtsComponentState(mts.mtsComponentState.ACTIVE))
time.sleep(1.0)

print 'waiting for provided interfaces to connect...'
while not bool(stealth.RequiredForPointer.GetConnectedInterface()):
    Pointer = False
stealth.RequiredForPointer.UpdateFromC()
print 'After adding Pointer Interface'

while not bool(stealth.RequiredForFrame.GetConnectedInterface()):
    Frame = False
stealth.RequiredForFrame.UpdateFromC()
print 'After adding Frame Interface'    

while not bool(stealth.RequiredForRegistration.GetConnectedInterface()):
    Registration = False
stealth.RequiredForRegistration.UpdateFromC()
print 'After adding Registration Interface'    

pose = stealth.RequiredForPointer.GetPositionCartesian()
print 'Pointer: ', pose.Position()
pose = stealth.RequiredForFrame.GetMarkerCartesian()
print 'Frame: ', pose.Position()
transform = stealth.RequiredForRegistration.GetTransformation()
print 'Registration: ', transform

zero = mts.mtsDouble(0.0)
collector.StartCollection(zero)
print 'start collection'
time.sleep(5.0)
collector.StopCollection(zero)
print 'stop collection'
#manager.KillAll()
#manager.Cleanup()
print 'finished sawMedtronicStealthlinkClientComponent.py...'
