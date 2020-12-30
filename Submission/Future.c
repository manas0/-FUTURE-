#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

unsigned char gmult(unsigned char a, unsigned char b)
{
	unsigned char p = 0; /* the product of the multiplication */
	if(a>0x13){
		a = a^0x13;
	}
	
	if(b>0x13){
		b = b^0x13;
	}

	while (a && b) {
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

	for(int i = 0; i < 16; i++){
		state[i] = s_box[state[i]];
	}
}

void MixColumn(unsigned char* state)
{

	unsigned char M[4][4] = {   {0x08 , 0x09 , 0x01 , 0x08},
								{0x03 , 0x02 , 0x09 , 0x09},
								{0x02 , 0x03 , 0x08 , 0x09},
								{0x09 , 0x09 , 0x08 , 0x01} };

	unsigned char temp[16] = { 0x00 } ;

	for(int i = 0 ; i < 16 ; i += 4){
		for(int j = 0; j < 4 ; j++){
			for(int k = 0; k < 4 ; k++){
				temp[i+j] ^= gmult(M[j][k],state[i+k]);
			} 
		}
	}

	for(int i  = 0 ; i < 16 ; i++){
		state[i] = temp[i];
	}
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
	
	for(int i = 0; i < 16; i++){
		state[i] = tmp[i];
	}
}

void Not(unsigned char* state, int roundNum){
	int bitPos = 3;
	if(roundNum == 1 || roundNum == 5){
		bitPos = 2; //MSB = 1
	}else if(roundNum == 2 || roundNum == 6){
		bitPos = 1; //MSB = 2
	}else if(roundNum == 3 || roundNum == 7){
		bitPos = 0; //MSB = 3
	}else{
		bitPos = 3;
	}
	for(int i = 0; i < 16; i++){
		int newState = state[i];
		if(bitPos != 3){
			int bit = (newState&(1<<bitPos));
			newState = (bit == 0 ? newState + (1<<bitPos) : newState - (1<<bitPos));
		}
		state[i] = newState;
	}
} 

void AddRoundKey(unsigned char* state, unsigned char* roundKey)
{
	for(int i = 0; i < 16; i++){
		state[i] ^= roundKey[i];
	}
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
	messageToState(message, state);
	int numOfRounds = 9;
	AddRoundKey(state, key[0]);//Add Round Key
	
	for(int i = 0; i < numOfRounds; i++)
	{
		SubCell(state);
		MixColumn(state);
		ShiftRows(state);
		Not(state, i);
		AddRoundKey(state, key[i+1]);
	}
	
	//Final Round
	SubCell(state);
	ShiftRows(state);
	AddRoundKey(state, key[10]);

	//EncryptedMessage
	for(int i = 0; i < 8; i++){
		message[i] = (state[2*i]<<4) | state[2*i + 1];
	}
}

void keySchedule(unsigned char key[11][16], unsigned char* Inputkey, unsigned char* bitwiseKey){
	int idx = 0, idx1 = 0, idx2 = 64;
	for(int i = 0; i < 16; i++){
		for(int j = 7; j >= 0; j--){
			if((Inputkey[i]&(1 << j)) > 0)
				bitwiseKey[idx] = 1;
			else
				bitwiseKey[idx] = 0;
			idx++;
		}
	}
	int powe2[4] = {1, 2, 4, 8};
	for(int round = 0; round < 11; round++){
		if(round%2 == 0){
			idx1 = 0 + (round/2)*5;
			for(int j = 0; j < 16; j++){
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
	//Print key
	// for(int i = 0; i < 11; i++){
	// 	printf("Round %d ", i);
	// 	for(int j = 0; j < 16; j++){
	// 		printf("%d ", key[i][j]);
	// 	}
	// 	printf("\n");
	// }
}

void XOR(unsigned char* message, unsigned char* blockOutput){
	for(int i = 0; i < 8; i++){
		message[i] ^= blockOutput[i];
	}
}

void newKeyCalc (unsigned char* Inputkey, unsigned char* block, unsigned char* newKey){
	unsigned char bitwiseKey[128], blockbitwise[64];
	int idx = 0, cnt = 0, val = 0;
	for(int i = 0; i < 16; i++){
		for(int j = 7; j >= 0; j--){
			if((Inputkey[i]&(1 << j)) > 0)
				bitwiseKey[idx] = 1;
			else
				bitwiseKey[idx] = 0;
			idx++;
		}
	}
	idx = 0;
	for(int i = 0; i < 8; i++){
		for(int j = 7; j >= 0; j--){
			if((block[i]&(1 << j)) > 0)
				blockbitwise[idx] = 1;
			else
				blockbitwise[idx] = 0;
			idx++;
		}
	}
	idx = 0;
	for(int i = 1; i < 128; i += 2){
		bitwiseKey[i] ^= blockbitwise[idx];
		idx++;
	}
	idx = 0;
	for(int i = 0; i < 128; i++){
		val |= (bitwiseKey[i] == 1 ? (1<<(7-cnt)) : 0);
		cnt++;
		if(cnt == 8){
			newKey[idx] = val;
			idx++;
			val = 0, cnt = 0;
		}
	}
}

void solve(unsigned char* Inputkey, unsigned char* InpMessage, unsigned char* blockOutput){
	unsigned char key[11][16], bitwiseKey[128];
	keySchedule(key, Inputkey, bitwiseKey);
	unsigned char message[8];
	for(int i = 0; i < 8; i++){
		message[i] = InpMessage[i];
	}
	encrypt(message, key);
	for(int i = 0; i < 8; i++){
		blockOutput[i] = message[i];
	}
}

void padding(unsigned char* message, int valid, unsigned char* extra_block){// always add an extra block, so always call this function
	int zero_count = 128 - 7 - 8*valid;
	if(valid != 1){
		for(int i = valid ; i < 8 ; i++){
			message[i] = 0;
		}
	}
	for(int i = 0 ; i < 8 ; i++){
		extra_block[i] = 0;
	}
	extra_block[7] = zero_count; // last 7 bits are reserved for number of bits of zeros appended
}

int main()
{
	//unsigned char Inputkey[16] = {0 , 1 , 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};//Input Key
	unsigned char Inputkey[16];
	unsigned char newKey[16];
	unsigned char InitialVector[8];//Initial Vector(IV)

	FILE *IV;
	IV = fopen("InitialVector.txt","r");
	for(int i = 0; i < 8; i++){
		char c = getc(IV);
		InitialVector[i] = c;
	}

	FILE *IK;
	IK = fopen("InputKey.txt","r");
	for(int i = 0; i < 16; i++){
		char c = getc(IV);
		Inputkey[i] = c;
	}
	// FILE *IK;
	// IK = fopen("Inputkey.txt","r");
	// for(int i = 0; i < 16; i++){
	// 	fscanf(IK, "%u\n", Inputkey[i]);
	// }

	unsigned char blockOutput[8];
	solve(Inputkey, InitialVector, blockOutput);//B0

	FILE *f;
	FILE *fptr;
	f = fopen("input.txt","r");
	fptr = fopen("output.txt","w");
	unsigned char message[8], prevMessage[8], cr[8], cr1[8];
	int blocks = 0;
	char c = getc(f);
	int counter = 0;
	while (c != EOF){
		message[counter] = c;
		counter++;
		if(counter == 8){
			for(int i = 0; i < 8; i++){
				prevMessage[i] = message[i];
			}
			XOR(message, blockOutput);//C0
			for(int i = 0; i < 8; i++){
				cr[i] = message[i];
				fprintf(fptr, "%X\n", cr[i]);
			}
			blocks++;
			newKeyCalc(Inputkey, message, newKey);//newKey
			solve(newKey, blockOutput, blockOutput);
			counter = 0;
		}
		c = getc(f);
	}

	unsigned char paddedBlock[8];
	padding(message, counter, paddedBlock);//message is mr is in message, paddedBlock has number of zeros
	if(counter == 1){
		for(int i = 0; i < 8; i++){
			message[i] = prevMessage[i];
		}
	}else{
		//Get Cr after Padding
		for(int i = 0; i < 8; i++){
			cr[i] = blockOutput[i]^message[i];
			fprintf(fptr, "%X\n", cr[i]);
		}
	}
	//encrpyt paddedBlock


	unsigned char Nonce[8];
	for(int i = 0; i < 8; i++){
		Nonce[i] = InitialVector[i];
	}

	FILE *asData;
	asData = fopen("asData.txt","r");
	int nMax = 10000;
	int n = 0;
	unsigned char* AuthenticData;
	AuthenticData = (unsigned char*)malloc(nMax * sizeof(unsigned char)); 
	char cc = getc(asData);
	int idxx = 0;
	while (cc != EOF){
		*(AuthenticData + idxx) = cc;
		cc= getc(asData);
		idxx++;
		n++;
	}
	fclose(asData);
	//Nonce(64,8),mr(64,8),cr(64,8),ad(variable)
	
	unsigned char* authenticatedMessage;
	authenticatedMessage = (unsigned char*)malloc((n + 25)* sizeof(unsigned char)); 

	int idx = 0;
	for(int i = 0; i < 8; i++){
		authenticatedMessage[idx] = Nonce[i];
		idx++;
	}
	for(int i = 0; i < 8; i++){
		authenticatedMessage[idx] = message[i];
		idx++;
	}
	for(int i = 0; i < 8; i++){
		authenticatedMessage[idx] = cr[i];
		idx++;
	}
	for(int i = 0; i < n; i++){
		authenticatedMessage[idx] = AuthenticData[i];
		idx++;
	}

	//cr acts as IV
	unsigned char tag[8];
	solve(Inputkey, cr, blockOutput);//B10
	int flag = 0, authCnt = 0;
	unsigned char mssg[8];
	for(int i = 0; i < (n+24); i += 8){
		authCnt = 0;
		for(int j = 0; j < 8; j++){
			mssg[j] = authenticatedMessage[i+j];
			authCnt++;
			if((i+j) >= (n+24)){
				flag = 1;
				break;
			}
		}
		if(flag){
			break;
		}
		XOR(mssg, blockOutput);//C0
		solve(Inputkey, mssg, blockOutput);
	}
	for(int i = 0; i < 8; i++){
		tag[i] = blockOutput[i];
	}
	
	//Padding
	unsigned char paddedBlock1[8];
	if(flag){
		padding(mssg, authCnt, paddedBlock1);
		for(int i = 0; i < 8; i++){
			tag[i] = mssg[i]^blockOutput[i];
		}
	}
	//encrpyt paddedBlock

	//blockOutput is tag
	char taggg[] = "Tag:";
	fprintf(fptr, "%s\n", taggg);
	for(int i = 0; i < 8; i++){
		fprintf(fptr,"%x\n",tag[i]);
	}
	fclose(fptr);
	fclose(f);
	fclose(IV);
	return 0;
}
