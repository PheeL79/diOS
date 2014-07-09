#ifndef _REVISION_H_
#define _REVISION_H_

#define REVISION 476

typedef enum {
    VER_LBL_PRE_ALPHA = 1,
    VER_LBL_ALPHA,
    VER_LBL_BETA,
    VER_LBL_RELEASE_CANDIDATE,
    VER_LBL_RELEASE_TO_MANUFACTURING,
    VER_LBL_GENERAL_AVAILABILITY,
    VER_LBL_END_OF_LIFE
} VersionLabel;

const char ver_lbl[8][4] =
{
    {""},
    {"pa"},
    {"a"},
    {"b"},
    {"rc"},
    {"rtm"},
    {"ga"},
    {"eol"}
};
	
#endif // _REVISION_H_
