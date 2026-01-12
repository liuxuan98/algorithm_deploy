#ifndef _AUTOFIOLLICLE_H_
#define _AUTOFIOLLICLE_H_

#include "base/macros.h"

typedef void *FollicleAlgorithmHandle;

struct SINGLE_OVARY_PARAMS {
    enum { MAX_OVARY_BUF_LEN = 1024 * 2 };

    int vecEdgePtsX[MAX_OVARY_BUF_LEN] = {0};
    int vecEdgePtsY[MAX_OVARY_BUF_LEN] = {0};
    int nEdgePtsNum = 0;
    int nFollicleNumInOvary = 0;

    int ptD1StartX = 0;
    int ptD1StartY = 0;

    int ptD1EndX = 0;
    int ptD1EndY = 0;

    int ptD2StartX = 0;
    int ptD2StartY = 0;

    int ptD2EndX = 0;
    int ptD2EndY = 0;

    double OVARY_PERI = 0;
    double OVARY_AREA = 0;
    double OVARY_CIRCLE = 0;
};

struct SINGLE_FOLLICLE_PARAMS {
    enum { MAX_FOLLICLE_BUF_LEN = 1024 };

    int vecEdgePtsX[MAX_FOLLICLE_BUF_LEN] = {0};
    int vecEdgePtsY[MAX_FOLLICLE_BUF_LEN] = {0};

    int nEdgePtsNum = 0;

    int ptD1StartX = 0;
    int ptD1StartY = 0;

    int ptD1EndX = 0;
    int ptD1EndY = 0;

    int ptD2StartX = 0;
    int ptD2StartY = 0;

    int ptD2EndX = 0;
    int ptD2EndY = 0;

    double FOLLICLE_PERI = 0.;
    double FOLLICLE_AREA = 0.;
    double FOLLICLE_CIRCLE = 0.;
};

typedef struct FollicleOutParams {
    enum { MAX_FOLLICLE_NUM = 16 };
    enum { MAX_OVARY_NUM = 2 };

    int nOvaryNum = 0;
    int nFollicleNum = 0;

    SINGLE_FOLLICLE_PARAMS sFollicle[MAX_FOLLICLE_NUM];
    SINGLE_OVARY_PARAMS sOvary[MAX_OVARY_NUM];
} FollicleOutParams;

RS_API ErrorCode FollicleAlgorthmInit(const char *model_json,
                                      FollicleAlgorithmHandle *algor_handle);

RS_API ErrorCode FollicleAlgorthmProcess(FollicleAlgorithmHandle algor_handle,
                                         unsigned char *src_image_ptr,
                                         FollicleOutParams *out_params);
#endif