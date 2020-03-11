
#include <sawCANBus/osaCANopen.h>

#include <cisstCommon.h>
#include <cisstOSAbstraction/osaGetTime.h>
#include <cisstOSAbstraction/osaSleep.h>

#include <fcntl.h>

osaCANopen::osaCANopen( osaCANBus* candevice ) : 
  candevice( candevice ), 
  deviceopened( false ){}

osaCANopen::~osaCANopen()
{ Close(); }

osaCANopen::Errno osaCANopen::Open(void){

  if( candevice == NULL ){
    CMN_LOG_RUN_ERROR << "Failed to open CAN device." << std::endl;
    return osaCANopen::EFAILURE;
  }
  
  if( candevice->Open() != osaCANBus::ESUCCESS ){
    CMN_LOG_RUN_ERROR << "Failed to open CAN device." << std::endl;
    return osaCANopen::EFAILURE;
  }
  
  deviceopened = true;
  return osaCANopen::ESUCCESS;
}

osaCANopen::Errno osaCANopen::Close(void){
  if( candevice == NULL ){
    CMN_LOG_INIT_ERROR << "Failed to close CAN device." << std::endl;
    return osaCANopen::EFAILURE;
  }
  
  if( candevice->Close() != osaCANBus::ESUCCESS ){
    CMN_LOG_INIT_ERROR << "Failed to close CAN device." << std::endl;
    return osaCANopen::EFAILURE;
  }
  
  deviceopened = false;
  return osaCANopen::ESUCCESS;
}

osaCANopen::Errno osaCANopen::Read( CiA301::COBID& cobid,
				    CiA301::Object& object ){
   
   
  if( candevice == NULL ){
    CMN_LOG_INIT_ERROR << "Failed to read CAN frame." << std::endl;
    return osaCANopen::EFAILURE;
  }

  osaCANBusFrame frame;
  if( candevice->Recv( frame ) != osaCANBus::ESUCCESS ){
    CMN_LOG_INIT_ERROR << "Failed to read CAN frame." << std::endl;
    return osaCANopen::EFAILURE;
  }

  Unpack( frame, cobid, object );
   
  return osaCANopen::ESUCCESS;
}

osaCANopen::Errno osaCANopen::Write( CiA301::COBID cobid,
				     const CiA301::Object& object ){
   
   if( candevice == NULL ){
      CMN_LOG_INIT_ERROR << "Failed to write CAN frame." << std::endl;
     return osaCANopen::EFAILURE;
   }

   if( candevice->Send( Pack( cobid, object ) ) != osaCANBus::ESUCCESS ){
     CMN_LOG_INIT_ERROR << "Failed to write CAN frame." << std::endl;
     return osaCANopen::EFAILURE;
   }

   return osaCANopen::ESUCCESS;
}

// Pack
// 
osaCANBusFrame osaCANopen::Pack( CiA301::COBID cobid,
				const CiA301::Object& object )
{ return osaCANBusFrame( cobid, object.data ); }

// Unpack
// 
void osaCANopen::Unpack( const osaCANBusFrame& frame,
			 CiA301::COBID& cobid,
			 CiA301::Object& object ){

   const osaCANBusFrame::Data* src = frame.GetData();
   CiA301::Object::DataField dest( frame.GetLength() );
  
  for( size_t i=0; i<frame.GetLength(); i++ )
     { dest[i] = src[i]; }
   
  cobid = frame.GetID();
  object = CiA301::Object( dest );
	
}

