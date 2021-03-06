#include "blursuperfast.h"
#include <stdio.h>
#include <stdlib.h>
#include "color.h"

int Ymagine_blurSuperfast(unsigned char *pix,
					  int w, int h, int pitch,
					  int colormode,
					  int radius, int niter)
{
	unsigned char *dv = NULL;
	int *vbuf = NULL;
	unsigned char *rgba = NULL;
	int wm, hm;
	int wh;
	int div;
	unsigned char *r, *g, *b, *a;
	int rsum, gsum, bsum, asum;
	int x, y;
	int i, n;
	int p, p1, p2;
	int yi, zi, yw;
	int maxwh;
	int *vmin;
	int *vmax;
	int alpha = 255;
	int alpha2 = 255;
	int rc = YMAGINE_ERROR;
	int bpp;
	int roffset, goffset, boffset, aoffset;
	int premultiply = 0;

	if (radius <= 0 || niter <= 0) {
		return YMAGINE_OK;
	}
	if (w <= 0 || h <= 0) {
		return YMAGINE_OK;
	}

	switch (colormode) 
	{
	case VBITMAP_COLOR_RGBA:
		bpp = 4;
		roffset = 0;
		goffset = 1;
		boffset = 2;
		aoffset = 3;
		premultiply = 0;
		break;
	case VBITMAP_COLOR_rgbA:
		bpp = 4;
		roffset = 0;
		goffset = 1;
		boffset = 2;
		aoffset = 3;
		premultiply = 1;
		break;
	case VBITMAP_COLOR_ARGB:
		bpp = 4;
		roffset = 1;
		goffset = 2;
		boffset = 3;
		aoffset = 0;
		premultiply = 0;
		break;
	case VBITMAP_COLOR_Argb:
		bpp = 4;
		roffset = 1;
		goffset = 2;
		boffset = 3;
		aoffset = 0;
		premultiply = 1;
		break;
	case VBITMAP_COLOR_RGB:
		bpp = 3;
		roffset = 0;
		goffset = 1;
		boffset = 2;
		aoffset = -1;
		premultiply = 0;
		break;
	default:
		return YMAGINE_ERROR;
	}

	maxwh = MAX(w, h);
	wm = w - 1;
	hm = h - 1;
	wh = w * h;
	div = radius + radius + 1;

	/* TODO: dont recalculate if calling multiple time with same radius */
	dv = (unsigned char*) Ymem_malloc(256*div*sizeof(dv[0]));
	if (dv == NULL) {
		goto cleanup;
	}
	for (i = 0 ; i < 256 * div; i++) {
		dv[i] = (i / div);
	}

	vbuf = (int*) Ymem_malloc(maxwh * sizeof(vbuf[0]) * 2);
	if (vbuf == NULL) {
		goto cleanup;
	}
	rgba = Ymem_malloc(wh * 4);
	if (rgba == NULL) {
		goto cleanup;
	}

	r = rgba;
	g = r + wh;
	b = g + wh;
	a = b + wh;

	vmin = vbuf;
	vmax = vbuf + maxwh;

	for (n = 0; n < niter; n++) {
		yw = 0;

		/* Vertical pass */
		for (y = 0; y < h; y++) {
			rsum = 0;
			gsum = 0;
			bsum = 0;
			asum = 0;

			yi = y * pitch;
			zi = y * w;

			for (i = -radius; i <= radius; i++) {
				p = yi + (MIN(wm, MAX(i,0)) * bpp);
				if (aoffset >= 0) {
					alpha = pix[p + aoffset];
				}

				if (alpha == 255 || premultiply) {
					rsum += pix[p + roffset];
					gsum += pix[p + goffset];
					bsum += pix[p + boffset];
				} else if (alpha != 0) {
					rsum += (pix[p + roffset] * alpha) / 255;
					gsum += (pix[p + goffset] * alpha) / 255;
					bsum += (pix[p + boffset] * alpha) / 255;
				}
				asum += alpha;
			}

			for (x = 0; x < w; x++) {
				r[zi] = dv[rsum];
				g[zi] = dv[gsum];
				b[zi] = dv[bsum];
				a[zi] = dv[asum];

				if (y == 0) {
					vmin[x] = MIN(x+radius+1, wm);
					vmax[x] = MAX(x-radius, 0);
				}
				p1 = yw + (vmin[x] * bpp);
				p2 = yw + (vmax[x] * bpp);

				if (aoffset >= 0) {
					alpha = pix[p1 + aoffset];
					alpha2 = pix[p2 + aoffset];
				}
				if ((alpha == 255 && alpha2 == 255) || premultiply) {
					rsum += pix[p1 + roffset] - pix[p2 + roffset];
					gsum += pix[p1 + goffset] - pix[p2 + goffset];
					bsum += pix[p1 + boffset] - pix[p2 + boffset];
				} else {
					rsum += (pix[p1 + roffset] * alpha) / 255 - (pix[p2 + roffset] * alpha2) / 255;
					gsum += (pix[p1 + goffset] * alpha) / 255 - (pix[p2 + goffset] * alpha2) / 255;
					bsum += (pix[p1 + boffset] * alpha) / 255 - (pix[p2 + boffset] * alpha2) / 255;
				}
				asum += alpha - alpha2;

				zi++;
			}

			yw += pitch;
		}

		/* Horizontal pass */
		for (x = 0; x < w; x++) {
			rsum = 0;
			gsum = 0;
			bsum = 0;
			asum = 0;

			for (i = -radius; i <= radius; i++) {
				yi = MIN(MAX(0,i),hm) * w + x;
				rsum += r[yi];
				gsum += g[yi];
				bsum += b[yi];
				asum += a[yi];
			}

			yi = x * bpp;
			for (y = 0; y < h; y++) {
				pix[yi + roffset] = dv[rsum];
				pix[yi + goffset] = dv[gsum];
				pix[yi + boffset] = dv[bsum];
				if (aoffset >= 0) {
					pix[yi + aoffset] = dv[asum];
				}

				if (x == 0) {
					vmin[y] = MIN(y+radius+1, hm)*w;
					vmax[y] = MAX(y-radius, 0)*w;
				}
				p1 = x + vmin[y];
				p2 = x + vmax[y];

				if (aoffset >= 0) {
					alpha = a[p1];
					alpha2 = a[p2];
				}

				rsum += r[p1] - r[p2];
				gsum += g[p1] - g[p2];
				bsum += b[p1] - b[p2];
				if (alpha != alpha2) {
					asum += alpha - alpha2;
				}

				yi += pitch;
			}
		}
	}

	rc = YMAGINE_OK;

cleanup:
	if (rgba != NULL) {
		Ymem_free(rgba);
		rgba = NULL;
	}
	if (vbuf != NULL) {
		Ymem_free(vbuf);
		vbuf = NULL;
	}
	if (dv != NULL) {
		Ymem_free(dv);
		dv = NULL;
	}

	return rc;
}



int Ymagine_blurBuffer(unsigned char *pix,
				   int w, int h, int pitch, int colormode,
				   int radius)
{
	int niter;

	if (radius <= 0) {
		return YMAGINE_OK;
	}

	niter = 1;
	while ((niter + 1) * (niter + 1) < radius && niter < 4) {
		niter++;
	}

	return Ymagine_blurSuperfast(pix, w, h, pitch, colormode, radius, niter);
}



