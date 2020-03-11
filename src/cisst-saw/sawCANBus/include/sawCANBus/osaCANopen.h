
#ifndef _osaCANopen_h
#define _osaCANopen_h

#include <sawCANBus/osaCANBus.h>
#include <sawCANBus/osaCANBusFrame.h>
#include <sawCANBus/CiA301.h>
#include <sawCANBus/CiA402.h>
#include <sawCANBus/sawCANBusExport.h>

class CISST_EXPORT osaCANopen {

 public:
   
   enum Errno
   {
     ESUCCESS = 0x00,
     EFAILURE = 0x01
   };

 private:
   
   //! This is the CAN device used to implement the CANopen protocol
   osaCANBus* candevice;
   bool deviceopened;
   
   osaCANBusFrame Pack( CiA301::COBID cobid, const CiA301::Object& object );
   void Unpack( const osaCANBusFrame& frame, 
		CiA301::COBID& cobid,
		CiA301::Object& object );
   
 public:
   
   osaCANopen( osaCANBus* can );
   ~osaCANopen();
   
   osaCANopen::Errno Open( void );
   osaCANopen::Errno Close( void );

   /*! \brief Read CAN message from CAN driver */
   osaCANopen::Errno Read( CiA301::COBID& cobid, CiA301::Object& object );

   /*! \brief Write CAN frame. */
   osaCANopen::Errno Write( CiA301::COBID cobid, const CiA301::Object& object );

   bool IsOpen(void) const { return deviceopened; }
   
};

#endif


