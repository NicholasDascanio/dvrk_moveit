#include <sawOpenDynamicsEngine/osaODEBarrettHand.h>
#include <sawOpenDynamicsEngine/osaODEManipulator.h>

//! Barrett Hand constructor
osaODEBarrettHand::osaODEBarrettHand( const std::string& palmmodel,
				      const std::string& metacarpalmodel,
				      const std::string& proximalmodel,
				      const std::string& intermediatemodel,
				      osaODEWorld* world,
				      const vctFrame4x4<double>& Rtw0,
				      const std::string& f1f2filename,
				      const std::string& f3filename ) : 
  osaOSGBH( Rtw0, std::string() ){
  
  // phoney nertia tensor for that palm (cylinder)
  vctFixedSizeMatrix<double,3,3> I(0.0);
  I[0][0] = ( 0.04*0.04 + 0.05*0.05 ) / 3.0;
  I[1][1] = ( 0.02*0.02 + 0.05*0.05 ) / 3.0;
  I[2][2] = ( 0.02*0.02 + 0.04*0.04 ) / 3.0;

  // Create the transformation of the base
  vctFrame4x4<double> Rt0b( vctMatrixRotation3<double>(),
			    vctFixedSizeVector<double,3>( 0.0, 0.0, 0.091 ) );
  vctFrame4x4<double> Rtwb = Rtw0 * Rt0b;

  // Create the body for the base (palm)
  osg::ref_ptr<osaODEBody> odebase = NULL;
  odebase = new osaODEBody( palmmodel,               // geometry
			    world,                   // the world
			    Rtwb,                    // position/orientation
			    1.17,                    // mass
			    vctFixedSizeVector<double,3>(0.0, 0.0, -0.025),
			    I,                       // iniertia tensor
			    1.0,
			    1.0,
			    "",
			    (osaOSGWorld*)this );  // ODE world space
  this->base = odebase;
  

  std::vector<std::string> modelsf1f2;
  modelsf1f2.push_back( metacarpalmodel );
  modelsf1f2.push_back( proximalmodel );
  modelsf1f2.push_back( intermediatemodel );

  vctFrame4x4<double> Rtbf1( vctMatrixRotation3<double>(),
			     vctFixedSizeVector<double,3>( 0.0,  0.025, -0.0095) );
  vctFrame4x4<double> Rtbf2( vctMatrixRotation3<double>(),
  			     vctFixedSizeVector<double,3>( 0.0, -0.025, -0.0095) );


  f1 = new osaODEManipulator( modelsf1f2,
			      world,
			      Rtwb*Rtbf1,
			      f1f2filename,
			      odebase.get(),
			      vctDynamicVector<double>( 3, 0.0 ) );
  f2 = new osaODEManipulator( modelsf1f2,
			      world,
			      Rtwb*Rtbf2,
			      f1f2filename,
			      odebase.get(),
			      vctDynamicVector<double>( 3, 0.0 ) );
  
  std::vector<std::string> modelsf3;
  modelsf3.push_back( proximalmodel );
  modelsf3.push_back( intermediatemodel );
  vctFrame4x4<double> Rtbf3( vctMatrixRotation3<double>( 1.0, 0.0, 0.0,
							 0.0, 0.0,-1.0,
							 0.0, 1.0, 0.0 ),
			     vctFixedSizeVector<double,3>( 0.05, 0.0,0.0 ) );

  
  f3 = new osaODEManipulator( modelsf3,
			      world,
			      Rtwb*Rtbf3,
			      f3filename,
			      odebase.get(),
			      vctDynamicVector<double>( 2, 0.0 ) );

  world->addChild( this );
}

//! Barrett Hand constructor  
osaODEBarrettHand::osaODEBarrettHand( const std::string& palmmodel,
			const std::string& metacarpalmodel,
			const std::string& proximalmodel,
			const std::string& intermediatemodel,
			osaODEWorld* world,
			const vctFrm3& Rtw0,
			const std::string& f1f2filename,
			const std::string& f3filename ):
  osaOSGBH( Rtw0, std::string() ){

  // phoney nertia tensor for that palm (cylinder)
  vctFixedSizeMatrix<double,3,3> I(0.0);
  I[0][0] = ( 0.04*0.04 + 0.05*0.05 ) / 3.0;
  I[1][1] = ( 0.02*0.02 + 0.05*0.05 ) / 3.0;
  I[2][2] = ( 0.02*0.02 + 0.04*0.04 ) / 3.0;

  // Create the transformation of the base
  vctFrm3 Rt0b( vctMatrixRotation3<double>(),
		vctFixedSizeVector<double,3>( 0.0, 0.0, 0.091 ) );
  vctFrm3 Rtwb = Rtw0 * Rt0b;

  // Create the body for the base (palm)
  osg::ref_ptr<osaODEBody> odebase = NULL;
  odebase = new osaODEBody( palmmodel,               // geometry
			    world,                   // the world
			    Rtwb,                    // position/orientation
			    1.17,                    // mass
			    vctFixedSizeVector<double,3>(0.0, 0.0, -0.025),
			    I,                       // iniertia tensor
			    1.0,
			    1.0,
			    "",
			    (osaOSGWorld*)this );  // ODE world space
  this->base = odebase;


  std::vector<std::string> modelsf1f2;
  modelsf1f2.push_back( metacarpalmodel );
  modelsf1f2.push_back( proximalmodel );
  modelsf1f2.push_back( intermediatemodel );

  vctFrm3 Rtbf1( vctMatrixRotation3<double>(),
		 vctFixedSizeVector<double,3>( 0.0,  0.026,-0.01) );
  vctFrm3 Rtbf2( vctMatrixRotation3<double>(),
		 vctFixedSizeVector<double,3>( 0.0, -0.026,-0.01) );
  
  f1 = new osaODEManipulator( modelsf1f2,
				world,
				Rtwb*Rtbf1,
				f1f2filename,
				odebase.get(),
				vctDynamicVector<double>( 3, 0.0 ) );

  f2 = new osaODEManipulator( modelsf1f2,
				world,
				Rtwb*Rtbf2,
				f1f2filename,
				odebase.get(),
				vctDynamicVector<double>( 3, 0.0 ) );

  std::vector<std::string> modelsf3;
  modelsf3.push_back( proximalmodel );
  modelsf3.push_back( intermediatemodel );
  vctFrm3 Rtbf3( vctMatrixRotation3<double>( 1.0, 0.0, 0.0,
					     0.0, 0.0,-1.0,
					     0.0, 1.0, 0.0 ),
		 vctFixedSizeVector<double,3>( 0.05, 0.0,0.0 ) );
  
  f3 = new osaODEManipulator( modelsf3,
				world,
				Rtwb*Rtbf3,
				f3filename,
				odebase.get(),
				vctDynamicVector<double>( 2, 0.0 ) );

  world->addChild( this );

}

  
osaODEBarrettHand::~osaODEBarrettHand(){}


