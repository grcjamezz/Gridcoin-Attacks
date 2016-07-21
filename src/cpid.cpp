/* 

CPID HASHING ALGORITHM - GRIDCOIN - ROB HALFORD - 10-21-2014

*/
 

#include "cpid.h"

#include <cstdio>

std::string YesNo(bool bin);

// Constants for CPID Transform routine.
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21
 
///////////////////////////////////////////////
 
// F, G, H and I are basic CPID functions.
inline CPID::uint4 CPID::F(uint4 x, uint4 y, uint4 z) 
{
  return ((x&y) | (~x&z));
}
 
inline CPID::uint4 CPID::G(uint4 x, uint4 y, uint4 z) 
{
  return ((x&z) | (y&~z));
}
 
inline CPID::uint4 CPID::H(uint4 x, uint4 y, uint4 z) {
  return (x^y^z);
}
 
inline CPID::uint4 CPID::I(uint4 x, uint4 y, uint4 z) {
  return y ^ (x | ~z);
}
 
// rotate_left rotates x left n bits.
inline CPID::uint4 CPID::rotate_left(uint4 x, int n) 
{
  return (x << n) | (x >> (32-n));
}
 
inline CPID::uint4 CPID::rotate_right(uint4 x, int n)
{
   // In n>>d, first d bits are 0. To put last 3 bits of at first, do bitwise or of n>>d with n <<(INT_BITS - d) 
   return (x >> n)|(x << (32 - n));
}


// rotate_left rotates x left n bits.
inline CPID::uint4 CPID::rotate_left8(int x, int n) 
{
  return (x << n) | (x >> (8-n));
}
 
inline CPID::uint4 CPID::rotate_right8(int x, int n)
{
   // In n>>d, first d bits are 0. To put last 3 bits of at first, do bitwise or of n>>d with n <<(INT_BITS - d) 
   return (x >> n)|(x << (8 - n));
}


// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
inline void CPID::FF(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac) 
{
  a = rotate_left(a+ F(b,c,d) + x + ac, s) + b;
}
 
inline void CPID::GG(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac) {
  a = rotate_left(a + G(b,c,d) + x + ac, s) + b;
}
 
inline void CPID::HH(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac) {
  a = rotate_left(a + H(b,c,d) + x + ac, s) + b;
}
 
inline void CPID::II(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac) {
  a = rotate_left(a + I(b,c,d) + x + ac, s) + b;
}
 
//////////////////////////////////////////////
 
// default constructor, just initailize
CPID::CPID()
{
  init();
}
 
//////////////////////////////////////////////
 
// constructor to compute CPID for string and finalize it right away
CPID::CPID(std::string text)
{
  init();
  update(text.c_str(), text.length());
  finalize();
}

//Compute a cpid given a blockhash and block-cpid 
//(If Long CPID was not generated by supplied block, new cpid will be invalid)
CPID::CPID(std::string text, int entropybit,uint256 hash_block)
{
  init();
  entropybit++;
  printf("a8.");
  update5(text,hash_block);
  printf("a9.");
  finalize();
  printf("a10.");
}

template< typename T >
std::string ByteToHex( T i )
{
  std::stringstream stream;
  stream << std::setfill ('0') << std::setw(2) 
         << std::hex << i;
  return stream.str();
}


std::string CPID::HashKey(std::string email1, std::string bpk1)
{
	 boost::algorithm::to_lower(bpk1);
	 boost::algorithm::to_lower(email1);
	 boinc_hash_new = bpk1+email1;
	 CPID c = CPID(boinc_hash_new);
	 std::string non_finalized = "";
	 non_finalized = c.hexdigest();
	 return non_finalized;
}


int BitwiseCount(std::string str, int pos)
{
	char ch;
	if (pos < (int)str.length())
	{
		ch = str.at(pos);
		int asc = (int)ch;
		if (asc > 47 && asc < 71) asc=asc-47;
		return asc;
	}
	return 1;
}

std::string HashHex(uint256 blockhash)
{
	CPID c2 = CPID(blockhash.GetHex());
	std::string shash = c2.hexdigest();
	return shash;
}


std::string ROR(std::string blockhash, int iPos, std::string hash)
{
	if (iPos <= (int)hash.length()-1)
	{
	    int asc1 = (int)hash.at(iPos);
		int rorcount = BitwiseCount(blockhash, iPos);
		std::string hex = ByteToHex(asc1+rorcount);
		return hex;
	}
	return "00";
}


std::string CPID::CPID_V2(std::string email1, std::string bpk1, uint256 block_hash)
{
	std::string non_finalized = HashKey(email1,bpk1);
	std::string digest = Update6(non_finalized, block_hash);
    //printf("CPID_V2 DEBUG: email1 %s bpk1 %s non_finalized %s cpidv2 %s\n",email1.c_str(),bpk1.c_str(),non_finalized.c_str(),digest.c_str());
    return digest;
}




//////////////////////////////
 
void CPID::init()
{
  finalized=false;
  
  count[0] = 0;
  count[1] = 0;
 
  // load magic initialization constants.
  state[0] = 0x67452301;
  state[1] = 0xefcdab89;
  state[2] = 0x98badcfe;
  state[3] = 0x10325476;
}
 
//////////////////////////////
 
// decodes input (unsigned char) into output (uint4). Assumes len is a multiple of 4.
void CPID::decode(uint4 output[], const uint1 input[], size_type len)
{
  for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((uint4)input[j]) | (((uint4)input[j+1]) << 8) |
      (((uint4)input[j+2]) << 16) | (((uint4)input[j+3]) << 24);
}
 
//////////////////////////////
 
// encodes input (uint4) into output (unsigned char). Assumes len is
// a multiple of 4.

//md5
void CPID::encode(uint1 output[], const uint4 input[], size_type len)
{
  for (size_type i = 0, j = 0; j < len; i++, j += 4) {
    output[j] = input[i] & 0xff;
    output[j+1] = (input[i] >> 8) & 0xff;
    output[j+2] = (input[i] >> 16) & 0xff;
    output[j+3] = (input[i] >> 24) & 0xff;
  }
}
 
//////////////////////////////
 
// apply CPID algo on a block

//md5
void CPID::transform(const uint1 block[blocksize])
{
  uint4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];
  decode (x, block, blocksize);
 
  // Round 1 
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); // 1 
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); // 2 
  FF (c, d, a, b, x[ 2], S13, 0x242070db); // 3 
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); // 4 
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); // 5 
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); // 6 
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); // 7 
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); // 8 
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); // 9 
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); // 10 
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); // 11 
  FF (b, c, d, a, x[11], S14, 0x895cd7be); // 12 
  FF (a, b, c, d, x[12], S11, 0x6b901122); // 13 
  FF (d, a, b, c, x[13], S12, 0xfd987193); // 14 
  FF (c, d, a, b, x[14], S13, 0xa679438e); // 15 
  FF (b, c, d, a, x[15], S14, 0x49b40821); // 16 
 
  // Round 2 
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); // 17 
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); // 18 
  GG (c, d, a, b, x[11], S23, 0x265e5a51); // 19 
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); // 20 
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); // 21 
  GG (d, a, b, c, x[10], S22,  0x2441453); // 22 
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); // 23 
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); // 24 
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); // 25 
  GG (d, a, b, c, x[14], S22, 0xc33707d6); // 26 
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); // 27 
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); // 28 
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); // 29 
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); // 30 
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); // 31 
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); // 32 
 
  // Round 3 
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); // 33 
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); // 34 
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); // 35 
  HH (b, c, d, a, x[14], S34, 0xfde5380c); // 36 
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); // 37 
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); // 38 
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); // 39 
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); // 40 
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); // 41 
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); // 42 
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); // 43 
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); // 44 
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); // 45 
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); // 46 
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); // 47 
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); // 48 
 
  // Round 4 
  II (a, b, c, d, x[ 0], S41, 0xf4292244); // 49 
  II (d, a, b, c, x[ 7], S42, 0x432aff97); // 50 
  II (c, d, a, b, x[14], S43, 0xab9423a7); // 51 
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); // 52 
  II (a, b, c, d, x[12], S41, 0x655b59c3); // 53 
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); // 54 
  II (c, d, a, b, x[10], S43, 0xffeff47d); // 55 
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); // 56 
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); // 57 
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); // 58 
  II (c, d, a, b, x[ 6], S43, 0xa3014314); // 59 
  II (b, c, d, a, x[13], S44, 0x4e0811a1); // 60 
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); // 61 
  II (d, a, b, c, x[11], S42, 0xbd3af235); // 62 
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); // 63 
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); // 64 
 
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
 
  // Zeroize sensitive information.
  memset(x, 0, sizeof x);
}
 
//////////////////////////////
 
// CPID block update operation. Continues a CPID message-digest
// operation, processing another message block

//md5
void CPID::update(const unsigned char input[], size_type length)
{
  // compute number of bytes mod 64
  size_type index = count[0] / 8 % blocksize;
 
  // Update number of bits
  if ((count[0] += (length << 3)) < (length << 3))
    count[1]++;
  count[1] += (length >> 29);
 
  // number of bytes we need to fill in buffer
  size_type firstpart = 64 - index;
 
  size_type i;
 
  // transform as many times as possible.
  if (length >= firstpart)
  {
    // fill buffer first, transform
    memcpy(&buffer[index], input, firstpart);
    transform(buffer);
 
    // transform chunks of blocksize (64 bytes)
    for (i = firstpart; i + blocksize <= length; i += blocksize)
      transform(&input[i]);
 
    index = 0;
  }
  else
    i = 0;
 
  // buffer remaining input
  memcpy(&buffer[index], &input[i], length-i);
}
 
//////////////////////////////
 
// for convenience provide a version with signed char
void CPID::update(const char input[], size_type length)
{
  update((const unsigned char*)input, length);
}




int HexToByte(std::string hex)
{
	int x = 0;
	std::stringstream ss;
	ss << std::hex << hex;
	ss >> x;
	return x;

}



int ROL(std::string blockhash, int iPos, std::string hash, int hexpos)
{
	    std::string cpid3 = "";
		if (iPos <= (int)hash.length()-1)
		{
			std::string hex = hash.substr(iPos,2);
			int rorcount = BitwiseCount(blockhash,hexpos);
			int b  = HexToByte(hex)-rorcount;
			if (b >= 0)
			{
				return b;
			}
		}
		return HexToByte("00");
}


std::string CPID::Update6(std::string non_finalized, uint256 block_hash)
{
    std::string shash = HashHex(block_hash);
    //printf("Update6 Debug: boinc_hash_new %s\n",boinc_hash_new.c_str());
    for (int i = 0; i < (int)boinc_hash_new.length(); i++)
    {
	 	non_finalized += ROR(shash,i,boinc_hash_new);
    }
	return non_finalized;
}

std::string Update7(std::string longcpid,uint256 hash_block)
{
	std::string shash = HashHex(hash_block);
	int hexpos = 0;
	std::string non_finalized = "";

	//unsigned char* input = new unsigned char[(longcpid.length()/2)+1];
	for (int i1 = 0; i1 < (int)longcpid.length(); i1 = i1 + 2)
    {
     		non_finalized +=  ROL(shash,i1,longcpid,hexpos);
			hexpos++;
    }

	CPID c7 = 	  CPID(non_finalized);
	std::string hexstring = c7.hexdigest();

	return hexstring;

}

std::string ReverseCPIDv2(std::string longcpid,uint256 hash_block)
{
	std::string shash = HashHex(hash_block);
	int hexpos = 0;
	std::string non_finalized = "";

	//unsigned char* input = new unsigned char[(longcpid.length()/2)+1];
	for (int i1 = 0; i1 < (int)longcpid.length(); i1 = i1 + 2)
    {
     		non_finalized +=  ROL(shash,i1,longcpid,hexpos);
			hexpos++;
    }
	return(non_finalized);

}


void CPID::update5(std::string longcpid,uint256 hash_block)
{
	std::string shash = HashHex(hash_block);
	int hexpos = 0;
	unsigned char* input = new unsigned char[(longcpid.length()/2)+1];
	for (int i1 = 0; i1 < (int)longcpid.length(); i1 = i1 + 2)
    {
			input[hexpos] = ROL(shash,i1,longcpid,hexpos);
			hexpos++;
    }
	
    input[longcpid.length()/2+1]=0;
    size_type length = longcpid.length()/2;

    // compute number of bytes mod 64
    size_type index = count[0] / 8 % blocksize;
    // Update number of bits
    if ((count[0] += (length << 3)) < (length << 3))
      count[1]++;
    count[1] += (length >> 29);
 
    // number of bytes we need to fill in buffer
    size_type firstpart = 64 - index;
    size_type i;
 
    // transform as many times as possible.
    if (length >= firstpart)
    {
		// fill buffer first, transform
		memcpy(&buffer[index], input, firstpart);
		transform(buffer);
 
		// transform chunks of blocksize (64 bytes)
		for (i = firstpart; i + blocksize <= length; i += blocksize)
		  transform(&input[i]);
 
		index = 0;
    }
    else
         i = 0;
 
    // buffer remaining input
    memcpy(&buffer[index], &input[i], length-i);
}


 
//////////////////////////////
 
// CPID finalization. Ends a CPID message-digest operation, writing the
// the message digest and zeroizing the context.
//md5
CPID& CPID::finalize()
{
  static unsigned char padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
 
  if (!finalized) {
    // Save number of bits
    unsigned char bits[8];
    encode(bits, count, 8);
 
    // pad out to 56 mod 64.
    size_type index = count[0] / 8 % 64;
    size_type padLen = (index < 56) ? (56 - index) : (120 - index);
    update(padding, padLen);
 
    // Append length (before padding)
    update(bits, 8);
 
    // Store state in digest
    encode(digest, state, 16);
 
    // Zeroize sensitive information.
    memset(buffer, 0, sizeof buffer);
    memset(count, 0, sizeof count);
 
    finalized=true;
  }
 
  return *this;
}





//////////////////////////////
 
// return hex representation of digest as string
std::string CPID::hexdigest() const
{
  if (!finalized)
  {
    return "00000000000000000000000000000000";
  }
 
  
  char buf[33];
  for (int i=0; i<16; i++)
    sprintf(buf+i*2, "%02x", digest[i]);
  buf[32]=0;
  return std::string(buf);
}
 
//////////////////////////////


template< typename T >
std::string LongToHex( T i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << i;
  return stream.str();
}





std::string CPID::boincdigest(std::string email, std::string bpk, uint256 hash_block)
{
  //Provides the 16 byte Boinc CPID (Netsoft compatible) contained in hash buffer

  if (!finalized)
    return "";


  char buf[16];
  for (int i=0; i<16; i++)
  {
		sprintf(buf+i*2, "%02x", digest[i]);
  }
  char ch;
  std::string non_finalized(buf);
  std::string shash =  HashHex(hash_block);
  
  std::string debug = "";

  
  boost::algorithm::to_lower(bpk);
  boost::algorithm::to_lower(email);
  std::string cpid_non = bpk+email;

  for (int i = 0; i < (int)cpid_non.length(); i++)
  {
	 	non_finalized += ROR(shash,i,cpid_non);
  }
  printf(".4 em %s bpk %s out %s",email.c_str(),bpk.c_str(),non_finalized.c_str());
  return non_finalized;

}


bool CompareCPID(std::string usercpid, std::string longcpid, uint256 blockhash)
{
	   if (longcpid.length() < 34) return false;
	   std::string cpid1 = longcpid.substr(0,32);
	   std::string cpid2 = longcpid.substr(32,longcpid.length()-31);
	   std::string shash =  HashHex(blockhash);
	   //	   CPID c = CPID(cpid2,0,blockhash);
	   std::string shortcpid = Update7(cpid2,blockhash);
	   if (shortcpid=="") return false;
	   if (fDebug) printf("shortcpid %s, cpid1 %s, usercpid %s \r\n",shortcpid.c_str(),cpid1.c_str(),usercpid.c_str());
	   if (shortcpid == cpid1 && cpid1==usercpid && shortcpid == usercpid) return true;
	   if (fDebug) printf("shortcpid %s, cpid1 %s, usercpid %s \r\n",shortcpid.c_str(),cpid1.c_str(),usercpid.c_str());
	   return false;
}


std::ostream& operator<<(std::ostream& out, CPID CPID)
{
  return out << CPID.hexdigest();
}
 
//////////////////////////////




bool CPID_IsCPIDValid(std::string cpid1, std::string longcpid, uint256 blockhash)
{
		if (cpid1.empty()) return false;
		if (longcpid.empty()) return false;
		if (longcpid.length() < 32) return false;
		if (cpid1.length() < 5) return false;
		if (cpid1=="INVESTOR" || longcpid=="INVESTOR") return true;
		if (cpid1.length() == 0 || longcpid.length() == 0)
		{
			printf("NULL Cpid received\r\n");
			return false;
		}
		if (longcpid.length() < 5) return false;
		//if (fDebug) printf("Comparing user cpid %s, longcpid %s\r\n",cpid1.c_str(),longcpid.c_str());
		bool compared = CompareCPID(cpid1,longcpid,blockhash);
		printf("?6a");
		return compared;
}



