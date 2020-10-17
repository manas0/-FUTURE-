#include <stdio.h>
#include <stdint.h>

unsigned char gmult(unsigned char a, unsigned char b)
{
	unsigned char p = 0; /* the product of the multiplication */
	if(a>0x13)
		a = a^0x13;
	
	if(b>0x13)
		b = b^0x13;

	while (a && b) 
	{
    	if (b & 1) /* if b is odd, then add the corresponding a to p (final product = sum of all a's corresponding to odd b's) */
            p ^= a; /* since we're in GF(2^m), addition is an XOR */
        
        if (a & 0x80) /* GF modulo: if a >= 128, then it will overflow when shifted left, so reduce */
            a = (a << 1) ^ 0x13; /* XOR with the primitive polynomial x^4 + x + 1 (0b_0001_011) – you can change it but it must be irreducible */
        else
            a <<= 1; /* equivalent to a*2 */
        
        b >>= 1; /* equivalent to b // 2 */
	}

	return p;
}

void KeyExpansion()
{

}

void InitialRound()
{

}

void SubCell(unsigned char* state)
{

	unsigned char s_box[] =   { 0x01 , 0x03 , 0x00 , 0x02 , 
								0x07 , 0x0e , 0x04 , 0x0d , 
								0x09 , 0x0a , 0x0c , 0x06 , 
								0x0f , 0x05 , 0x08 , 0x0b };

	for(int i = 0; i < 16; i++)
		state[i] = s_box[state[i]];
	
	for(int i = 0; i < 16; i++)
		printf("%d\n", state[i]);

}

void MixColumn(unsigned char* state)
{

	unsigned char M[4][4] = {   {0x08 , 0x09 , 0x01 , 0x08},
								{0x03 , 0x02 , 0x09 , 0x09},
								{0x02 , 0x03 , 0x08 , 0x09},
								{0x09 , 0x09 , 0x08 , 0x01} };

	unsigned char temp[16] = { 0x00 } ;

	for(int i = 0 ; i < 16 ; i += 4)
	{
		for(int j = 0; j < 4 ; j++)
		{
			for(int k = 0; k < 4 ; k++)
			{
				temp[i+j] ^= gmult(M[j][k],state[i+k]);
			} 
		}
	}

	for(int i  = 0 ; i < 16 ; i++)
		state[i] = temp[i];
 

}


void ShiftRows(unsigned char* state)
{
	unsigned char tmp[16];
	
	tmp[0] = state[0];
	tmp[1] = state[5];
	tmp[2] = state[10];
	tmp[3] = state[15];
	tmp[4] = state[4];
	tmp[5] = state[9];
	tmp[6] = state[14];
	tmp[7] = state[3];
	tmp[8] = state[8];
	tmp[9] = state[13];
	tmp[10] = state[2];
	tmp[11] = state[7];
	tmp[12] = state[12];
	tmp[13] = state[1];
	tmp[14] = state[6];
	tmp[15] = state[11];
	
	for(int i = 0; i < 16; i++)
		state[i] = tmp[i];
}


void AddRoundKey(unsigned char* state, unsigned char* roundKey)
{
	for(int i = 0; i < 16; i++)
		state[i] ^= roundKey[i];

}

void encrypt(unsigned char* message, unsigned char* key)
{
	unsigned char state[16];
	for(int i = 0; i < 16; i++)
		state[i] = message[i];
	
	int numOfRounds = 9;
	InitialRound(state, key);//Add Round Key
	
	for(int i = 0; i < numOfRounds; i++)
	{
		SubCell(state);
		MixColumn(state);
		ShiftRows(state);
		AddRoundKey(state, key);
	}
	
	//Final Round
	SubBytes(state);
	ShiftRows(state);
	AddRoundKey(state, key);

	//EncryptedMessage
	for(int i = 0; i < 16; i++)
		message[i] = state[i];
	
}

int main()
{
	unsigned char message[] = "This is a message we will encryt with AES";
	unsigned char key[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	encrypt(message, key);

	int originalLen = strlen((const char*)meassage);
	int lenOfPaddedMessage = originalLen;

	if(lenOfPaddedMessage%16 != 0)
		lenOfPaddedMessage = (lenOfPaddedMessage/16 + 1)*16;
	
	unsigned char* paddedMessage = new unsigned char[lenOfPaddedMessage];
	
	for(int i = 0; i < lenOfPaddedMessage; i++)
	{
		if(i >= originalLen)
			paddedMessage[i] = 0;
		
		else
			paddedMessage[i] = message[i];
	}

	//Encrypt padded Message
	for(int i = 0; i < lenOfPaddedMessage; i++)
		encrypt(paddedMessage + i, key);

	return 0;
}