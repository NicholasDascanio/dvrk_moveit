#ifndef __SAW_MINIMAL_PRM_H
#define __SAW_MINIMAL_PRM_H

typedef enum JointType {
    PRM_PRISMATIC,
    PRM_REVOLUTE
} prmJointType;

typedef std::vector<prmJointType> prmJointTypeVec;

#endif // ifndef __SAW_MINIMAL_PRM_H
