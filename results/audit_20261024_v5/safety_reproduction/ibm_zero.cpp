class CSegyIO {public: void IBM_to_FLOAT(int*,int*,int,int);};
void CSegyIO::IBM_to_FLOAT(int from[], int to[], int n, int endian)
{
	register int fconv, fmant, i, t;

	for (i = 0;i < n; ++i) 
	{
		fconv = from[i];
		/* if little endian, i.e. endian=0 do this */
		if (endian == 0) fconv = (fconv << 24) | ((fconv >> 24) & 0xff) |
			((fconv & 0xff00) << 8) | ((fconv & 0xff0000) >> 8);

		if (fconv)
		{
			fmant = 0x00ffffff & fconv;
			/* The next two lines were added by Toralf Foerster */
			/* to trap non-IBM format data i.e. conv=0 data  */

			t = (int) ((0x7f000000 & fconv) >> 22) - 130;
			while (!(fmant & 0x00800000)) { --t; fmant <<= 1; }
			if (t > 254) fconv = (0x80000000 & fconv) | 0x7f7fffff;
			else if (t <= 0) fconv = 0;
			else fconv =   (0x80000000 & fconv) | (t << 23)
				| (0x007fffff & fmant);
		}
		to[i] = fconv;
	}
	return;
}

int main(){CSegyIO io; int word=0x41000000,result=1;io.IBM_to_FLOAT(&word,&result,1,1);return result!=0;}
