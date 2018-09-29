/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

//2D median filter OpenCL example
kernel void filter(global uchar* pSrc, global uchar* pDst)
{

	const int x = get_global_id(0);
    const int y = get_global_id(1);

	const int iOffset = y * (uint)WIDTH;
    const int iPrev = iOffset - (uint)WIDTH;
    const int iNext = iOffset + (uint)WIDTH;

    // transfer pixels within median window to local variables
	uchar r0,r1,r2,r3,r4,r5,r6,r7,r8;
	r0 = pSrc[iPrev + x - 1];
    r1 = pSrc[iPrev + x];
    r2 = pSrc[iPrev + x + 1];

    r3 = pSrc[iOffset + x - 1];
    r4 = pSrc[iOffset + x];
    r5 = pSrc[iOffset + x + 1];

    r6 = pSrc[iNext + x - 1];
    r7 = pSrc[iNext + x];
    r8 = pSrc[iNext + x + 1];

    uchar uiResult = 0;

        // perform partial bitonic sort to find the median value
        uchar uiMin = min(r0, r1);
        uchar uiMax = max(r0, r1);
        r0 = uiMin;
        r1 = uiMax;

        uiMin = min(r3, r2);
        uiMax = max(r3, r2);
        r3 = uiMin;
        r2 = uiMax;

        uiMin = min(r2, r0);
        uiMax = max(r2, r0);
        r2 = uiMin;
        r0 = uiMax;

        uiMin = min(r3, r1);
        uiMax = max(r3, r1);
        r3 = uiMin;
        r1 = uiMax;

        uiMin = min(r1, r0);
        uiMax = max(r1, r0);
        r1 = uiMin;
        r0 = uiMax;

        uiMin = min(r3, r2);
        uiMax = max(r3, r2);
        r3 = uiMin;
        r2 = uiMax;

        uiMin = min(r5, r4);
        uiMax = max(r5, r4);
        r5 = uiMin;
        r4 = uiMax;

        uiMin = min(r7, r8);
        uiMax = max(r7, r8);
        r7 = uiMin;
        r8 = uiMax;

        uiMin = min(r6, r8);
        uiMax = max(r6, r8);
        r6 = uiMin;
        r8 = uiMax;

        uiMin = min(r6, r7);
        uiMax = max(r6, r7);
        r6 = uiMin;
        r7 = uiMax;

        uiMin = min(r4, r8);
        uiMax = max(r4, r8);
        r4 = uiMin;
        r8 = uiMax;

        uiMin = min(r4, r6);
        uiMax = max(r4, r6);
        r4 = uiMin;
        r6 = uiMax;

        uiMin = min(r5, r7);
        uiMax = max(r5, r7);
        r5 = uiMin;
        r7 = uiMax;

        uiMin = min(r4, r5);
        uiMax = max(r4, r5);
        r4 = uiMin;
        r5 = uiMax;

        uiMin = min(r6, r7);
        uiMax = max(r6, r7);
        r6 = uiMin;
        r7 = uiMax;

        uiMin = min(r0, r8);
        uiMax = max(r0, r8);
        r0 = uiMin;
        r8 = uiMax;

        r4 = max(r0, r4);
        r5 = max(r1, r5);

        r6 = max(r2, r6);
        r7 = max(r3, r7);

        r4 = min(r4, r6);
        r5 = min(r5, r7);

	pDst[iOffset + x] = (uchar)min(r4, r5);

}
