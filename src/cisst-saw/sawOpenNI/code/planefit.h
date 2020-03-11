/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: planefit.h 3193 2011-11-17 17:11:41Z bvagvol1 $

  Author(s):  Balazs Vagvolgyi
  Created on: 2011

  (C) Copyright 2006-2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

/*
This code is based partially on a plane fitting routine released in
'snippet' form by John W. Ratcliff mailto:jratcliffscarab@gmail.com
on March 22, 2006 under the MIT license.

** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#ifndef _PLANE_FIT_H
#define _PLANE_FIT_H

#include <cisstVector/vctDynamicVectorTypes.h>
#include <cisstVector/vctFixedSizeVectorTypes.h>
#include <math.h>


template <class _DATA_TYPE>
class PlaneFit
{
public:

    bool Calculate(vctDynamicVectorRef<vctFixedSizeVector<_DATA_TYPE, 3> > points,
                   vctDynamicVectorRef<_DATA_TYPE>                         weights,
                   vctFixedSizeVector<_DATA_TYPE, 4>                     & plane)
    {
        unsigned int vcount = points.size();
        if (vcount < 3) return false;

        unsigned int wcount = weights.size();
        if (wcount > 0 && vcount != wcount) wcount = 0;

        bool ret = false;

        _DATA_TYPE ox = 0, oy = 0, oz = 0, dx, dy, dz, nx, ny, nz, w, wtotal = 0;

        if (wcount) {
            for (unsigned int i = 0; i < vcount; i ++) {
                w = weights[i];
                wtotal += w;

                ox += points[i][0] * w;
                oy += points[i][1] * w;
                oz += points[i][2] * w;
            }
        }
        else {
            for (unsigned int i = 0; i < vcount; i ++) {
                wtotal += 1;

                ox += points[i][0];
                oy += points[i][1];
                oz += points[i][2];
            }
        }

        _DATA_TYPE recip = (_DATA_TYPE)1.0 / wtotal; // reciprocol of total weighting

        ox *= recip;
        oy *= recip;
        oz *= recip;

        _DATA_TYPE fSumXX = 0;
        _DATA_TYPE fSumXY = 0;
        _DATA_TYPE fSumXZ = 0;
        _DATA_TYPE fSumYY = 0;
        _DATA_TYPE fSumYZ = 0;
        _DATA_TYPE fSumZZ = 0;

        if (wcount) {
            for (unsigned int i = 0; i < vcount; i ++) {
                w = weights[i];

                // Apply vertex weighting
                dx = w * (points[i][0] - ox);
                dy = w * (points[i][1] - oy);
                dz = w * (points[i][2] - oz);

                // SSD
                fSumXX += dx * dx;
                fSumXY += dx * dy;
                fSumXZ += dx * dz;
                fSumYY += dy * dy;
                fSumYZ += dy * dz;
                fSumZZ += dz * dz;
            }
        }
        else {
            for (unsigned int i = 0; i < vcount; i ++) {
                // Apply vertex weighting
                dx = points[i][0] - ox;
                dy = points[i][1] - oy;
                dz = points[i][2] - oz;

                // SSD
                fSumXX += dx * dx;
                fSumXY += dx * dy;
                fSumXZ += dx * dz;
                fSumYY += dy * dy;
                fSumYZ += dy * dz;
                fSumZZ += dz * dz;
            }
        }

        fSumXX *= recip;
        fSumXY *= recip;
        fSumXZ *= recip;
        fSumYY *= recip;
        fSumYZ *= recip;
        fSumZZ *= recip;

        mElement[0][0] = fSumXX;
        mElement[0][1] = fSumXY;
        mElement[0][2] = fSumXZ;

        mElement[1][0] = fSumXY;
        mElement[1][1] = fSumYY;
        mElement[1][2] = fSumYZ;

        mElement[2][0] = fSumXZ;
        mElement[2][1] = fSumYZ;
        mElement[2][2] = fSumZZ;

        // compute eigenstuff, smallest eigenvalue is in last position
        DecrSortEigenStuff();

        nx = mElement[0][2];
        ny = mElement[1][2];
        nz = mElement[2][2];

        // the minimum energy
        plane[0] = nx;
        plane[1] = ny;
        plane[2] = nz;

        plane[3] = 0 - (nx * ox + ny * oy + nz * oz); // Dot product

        return ret;
    }

protected:

    void DecrSortEigenStuff()
    {
        Tridiagonal(); //diagonalize the matrix.
        QLAlgorithm();
        DecreasingSort();
        GuaranteeRotation();
    }

    void Tridiagonal()
    {
        _DATA_TYPE fM00 = mElement[0][0];
        _DATA_TYPE fM01 = mElement[0][1];
        _DATA_TYPE fM02 = mElement[0][2];
        _DATA_TYPE fM11 = mElement[1][1];
        _DATA_TYPE fM12 = mElement[1][2];
        _DATA_TYPE fM22 = mElement[2][2];

        m_afDiag[0] = fM00;
        m_afSubd[2] = 0;
        if (fM02 != (_DATA_TYPE)0.0)
        {
            _DATA_TYPE fLength = sqrt(fM01 * fM01 + fM02 * fM02);
            _DATA_TYPE fInvLength = ((_DATA_TYPE)1.0) / fLength;
            fM01 *= fInvLength;
            fM02 *= fInvLength;
            _DATA_TYPE fQ = ((_DATA_TYPE)2.0) * fM01 * fM12 + fM02 * (fM22 - fM11);
            m_afDiag[1] = fM11 + fM02 * fQ;
            m_afDiag[2] = fM22 - fM02 * fQ;
            m_afSubd[0] = fLength;
            m_afSubd[1] = fM12 - fM01 * fQ;
            mElement[0][0] = (_DATA_TYPE)1.0;
            mElement[0][1] = (_DATA_TYPE)0.0;
            mElement[0][2] = (_DATA_TYPE)0.0;
            mElement[1][0] = (_DATA_TYPE)0.0;
            mElement[1][1] = fM01;
            mElement[1][2] = fM02;
            mElement[2][0] = (_DATA_TYPE)0.0;
            mElement[2][1] = fM02;
            mElement[2][2] = -fM01;
            m_bIsRotation = false;
        }
        else
        {
            m_afDiag[1] = fM11;
            m_afDiag[2] = fM22;
            m_afSubd[0] = fM01;
            m_afSubd[1] = fM12;
            mElement[0][0] = (_DATA_TYPE)1.0;
            mElement[0][1] = (_DATA_TYPE)0.0;
            mElement[0][2] = (_DATA_TYPE)0.0;
            mElement[1][0] = (_DATA_TYPE)0.0;
            mElement[1][1] = (_DATA_TYPE)1.0;
            mElement[1][2] = (_DATA_TYPE)0.0;
            mElement[2][0] = (_DATA_TYPE)0.0;
            mElement[2][1] = (_DATA_TYPE)0.0;
            mElement[2][2] = (_DATA_TYPE)1.0;
            m_bIsRotation = true;
        }
    }

    bool QLAlgorithm()
    {
        const int iMaxIter = 32;

        for (int i0 = 0; i0 < 3; i0 ++)
        {
            int i1;
            for (i1 = 0; i1 < iMaxIter; i1 ++)
            {
                int i2;
                for (i2 = i0; i2 <= (3 - 2); i2 ++)
                {
                    _DATA_TYPE fTmp = fabs(m_afDiag[i2]) + fabs(m_afDiag[i2 + 1]);
                    if (fabs(m_afSubd[i2]) + fTmp == fTmp)
                        break;
                }
                if (i2 == i0)
                {
                    break;
                }

                _DATA_TYPE fG = (m_afDiag[i0 + 1] - m_afDiag[i0]) / (((_DATA_TYPE)2.0) * m_afSubd[i0]);
                _DATA_TYPE fR = sqrt(fG * fG + (_DATA_TYPE)1.0);
                if (fG < (_DATA_TYPE)0.0)
                {
                    fG = m_afDiag[i2] - m_afDiag[i0] + m_afSubd[i0] / (fG - fR);
                }
                else
                {
                    fG = m_afDiag[i2] - m_afDiag[i0] + m_afSubd[i0] / (fG + fR);
                }
                _DATA_TYPE fSin = (_DATA_TYPE)1.0, fCos = (_DATA_TYPE)1.0, fP = (_DATA_TYPE)0.0;
                for (int i3 = i2 - 1; i3 >= i0; i3 --)
                {
                    _DATA_TYPE fF = fSin * m_afSubd[i3];
                    _DATA_TYPE fB = fCos * m_afSubd[i3];
                    if (fabs(fF) >= fabs(fG))
                    {
                        fCos = fG/fF;
                        fR = sqrt(fCos * fCos + (_DATA_TYPE)1.0);
                        m_afSubd[i3 + 1] = fF * fR;
                        fSin = ((_DATA_TYPE)1.0) / fR;
                        fCos *= fSin;
                    }
                    else
                    {
                        fSin = fF / fG;
                        fR = sqrt(fSin * fSin + (_DATA_TYPE)1.0);
                        m_afSubd[i3 + 1] = fG * fR;
                        fCos = ((_DATA_TYPE)1.0) / fR;
                        fSin *= fCos;
                    }
                    fG = m_afDiag[i3+1] - fP;
                    fR = (m_afDiag[i3] - fG) * fSin + ((_DATA_TYPE)2.0) * fB * fCos;
                    fP = fSin * fR;
                    m_afDiag[i3 + 1] = fG + fP;
                    fG = fCos * fR - fB;
                    for (int i4 = 0; i4 < 3; i4 ++)
                    {
                        fF = mElement[i4][i3 + 1];
                        mElement[i4][i3 + 1] = fSin * mElement[i4][i3] + fCos * fF;
                        mElement[i4][i3] = fCos * mElement[i4][i3] - fSin * fF;
                    }
                }
                m_afDiag[i0] -= fP;
                m_afSubd[i0] = fG;
                m_afSubd[i2] = (_DATA_TYPE)0.0;
            }
            if (i1 == iMaxIter)
            {
                return false;
            }
        }
        return true;
    }

    void DecreasingSort()
    {
        //sort eigenvalues in decreasing order, e[0] >= ... >= e[iSize-1]
        for (int i0 = 0, i1; i0 <= 3 - 2; i0 ++)
        {
            // locate maximum eigenvalue
            i1 = i0;
            _DATA_TYPE fMax = m_afDiag[i1];
            int i2;
            for (i2 = i0 + 1; i2 < 3; i2 ++)
            {
                if (m_afDiag[i2] > fMax)
                {
                    i1 = i2;
                    fMax = m_afDiag[i1];
                }
            }

            if (i1 != i0)
            {
                // swap eigenvalues
                m_afDiag[i1] = m_afDiag[i0];
                m_afDiag[i0] = fMax;
                // swap eigenvectors
                for (i2 = 0; i2 < 3; i2 ++)
                {
                    _DATA_TYPE fTmp = mElement[i2][i0];
                    mElement[i2][i0] = mElement[i2][i1];
                    mElement[i2][i1] = fTmp;
                    m_bIsRotation = !m_bIsRotation;
                }
            }
        }
    }

    void GuaranteeRotation()
    {
        if (!m_bIsRotation)
        {
            // change sign on the first column
            for (int iRow = 0; iRow < 3; iRow ++)
            {
                mElement[iRow][0] = -mElement[iRow][0];
            }
        }
    }

    _DATA_TYPE mElement[3][3];
    _DATA_TYPE m_afDiag[3];
    _DATA_TYPE m_afSubd[3];
    bool m_bIsRotation;
};

#endif // _PLANE_FIT_H

