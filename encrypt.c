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
        
        if (a & 0x08) /* GF modulo: if a >= 8, then it will overflow when shifted left, so reduce */
            a = (a << 1) ^ 0x13; /* XOR with the primitive polynomial x^4 + x + 1 (0b_0001_011) */
        else
            a <<= 1; /* equivalent to a*2 */
        
        b >>= 1; /* equivalent to b // 2 */
	}

	return p;
}

void SubCell(unsigned char* state)
{

	unsigned char s_box[] =   { 0x01 , 0x03 , 0x00 , 0x02 , 
								0x07 , 0x0e , 0x04 , 0x0d , 
								0x09 , 0x0a , 0x0c , 0x06 , 
								0x0f , 0x05 , 0x08 , 0x0b };

	for(int i = 0; i < 16; i++)
		state[i] = s_box[state[i]];
	
	// for(int i = 0; i < 16; i++)
	// 	printf("%d\n", state[i]);

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
	tmp[1] = state[13];
	tmp[2] = state[10];
	tmp[3] = state[7];
	tmp[4] = state[4];
	tmp[5] = state[1];
	tmp[6] = state[14];
	tmp[7] = state[11];
	tmp[8] = state[8];
	tmp[9] = state[5];
	tmp[10] = state[2];
	tmp[11] = state[15];
	tmp[12] = state[12];
	tmp[13] = state[9];
	tmp[14] = state[6];
	tmp[15] = state[3];
	
	for(int i = 0; i < 16; i++)
		state[i] = tmp[i];
}

void AddRoundKey(unsigned char* state, unsigned char* roundKey)
{
	for(int i = 0; i < 16; i++)
		state[i] ^= roundKey[i];

}

void messageToState(unsigned char* message, unsigned char* state){
	for(int i = 0; i < 8; i++){
		state[2*i] = message[i]>>4;
		state[2*i+1] = message[i]%16;
	}
}

void encrypt(unsigned char* message, unsigned char key[11][16])
{
	unsigned char state[16] = {0};
	// for(int i = 0; i < 16; i++)
	// 	state[i] = message[i];
	messageToState(message, state);
	int numOfRounds = 9;
	AddRoundKey(state, key[0]);//Add Round Key
	
	for(int i = 0; i < numOfRounds; i++)
	{
		SubCell(state);
		MixColumn(state);
		ShiftRows(state);
		AddRoundKey(state, key[i+1]);
	}
	
	//Final Round
	SubCell(state);
	ShiftRows(state);
	AddRoundKey(state, key[10]);

	//EncryptedMessage
	for(int i = 0; i < 8; i++)
	{
		message[i] = (state[2*i]<<4) | state[2*i + 1];
	}
	
}

void keySchedule(unsigned char key[11][16], unsigned char* Inputkey, unsigned char* bitwiseKey){
	int idx = 0, idx1 = 0, idx2 = 64;
	// for(int i = 0; i < 16; i++){
	// 	printf("%c ", Inputkey[i]);
	// }
	// printf("\n");
	for(int i = 0; i < 16; i++){
		for(int j = 7; j >= 0; j--){
			if((Inputkey[i]&(1 << j)) > 0)
				bitwiseKey[idx] = 1;
			else
				bitwiseKey[idx] = 0;
			idx++;
		}
	}
	// printf("key: ");
	// for(int i = 0; i < 128; i++){
	// 	printf("%d %d\n", i, bitwiseKey[i]);
	// }
	// printf("\n");
	int powe2[4] = {1, 2, 4, 8};
	for(int round = 0; round < 11; round++){
		if(round%2 == 0){
			idx1 = 0 + (round/2)*5;
			for(int j = 0; j < 16; j++){
				// key[round][j] = bitwiseKey[idx1]*8 + bitwiseKey[idx1+1]*4 + bitwiseKey[idx1+2]*2 + bitwiseKey[idx1+3];
				key[round][j] = 0;
				for(int k = 3; k >= 0; k--){
					key[round][j] += bitwiseKey[idx1]*powe2[k];
					idx1++;
					idx1 %= 64;
				}
			}
		}else{
			idx2 = 64 + ((round-1)/2)*5;
			for(int j = 0; j < 16; j++){
				//key[round][j] = bitwiseKey[idx2]*8 + bitwiseKey[idx2+1]*4 + bitwiseKey[idx2+2]*2 + bitwiseKey[idx2+3];
				key[round][j] = 0;
				for(int k = 3; k >= 0; k--){
					key[round][j] += bitwiseKey[idx2]*powe2[k];
					idx2++;
					idx2 %= 128;
					if(idx2 == 0)idx2 = 64;
				}
			}
		}
	}
	for(int i = 0; i < 11; i++){
		printf("Round %d ", i);
		for(int j = 0; j < 16; j++){
			printf("%d ", key[i][j]);
		}
		printf("\n");
	}
}

int main()
{
	unsigned char message[] = "abcdefgh";
	unsigned char Inputkey[16] = {0 , 1 , 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	unsigned char key[11][16], bitwiseKey[128];
	keySchedule(key, Inputkey, bitwiseKey);
	encrypt(message, key);

	// int originalLen = strlen((const char*)message);
	// int lenOfPaddedMessage = originalLen;

	// if(lenOfPaddedMessage%16 != 0)
	// 	lenOfPaddedMessage = (lenOfPaddedMessage/16 + 1)*16;
	
	// unsigned char* paddedMessage = new unsigned char[lenOfPaddedMessage];
	
	// for(int i = 0; i < lenOfPaddedMessage; i++)
	// {
	// 	if(i >= originalLen)
	// 		paddedMessage[i] = 0;
		
	// 	else
	// 		paddedMessage[i] = message[i];
	// }

	// //Encrypt padded Message
	// for(int i = 0; i < lenOfPaddedMessage; i+=8)
	// 	encrypt(paddedMessage + i, key);

	printf("\n\n");
	for(int i = 0 ; i<8 ; i++)
		printf("%x\n",message[i]);

	return 0;
}
