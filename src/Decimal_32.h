#pragma once
#include <initializer_list>
#include <iostream>
typedef unsigned char utiny;
typedef signed char tiny;
#define big (size_t)0-(size_t)1
#define inrange(x, a, b) ((x) >= (a) && (x) < (b)) //EXCULSIVE OF B
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define diff(a, b) ((a) > (b) ? ((a)-(b)) : ((b)-(a)))

#define PI 3.1415926535897932384626433832795028841971693993751058209740445923078164062862089986280348253421170679811048086913282306647093844
#define B11111111 255 //some visual byte definitions
#define B11110000 240
#define B00001111 15
#define B00000000 0
#define B00000001 1
#define B00010000 16

class Decimal_32 {
private:
	utiny fraction_[31]; //supports up to 61 digits, 2 in each but 1 leading sign (1 means negative). Not fully compressed but for efficiency
	//Was considering removing leading bit from mantissa and using it as the sign, instead of taking off a digit, but decided against it
	utiny mantissa_; // ranges from 0 - 255. Should be 127 when {31}.{31} (to break up fairly), 97 when {1}.{61}, 158 when {61}.{0} [default], 0 when {-96}.{158}, and 255 when {159}.{-97}
	void writeToFraction_(int dig_i, utiny dig) { //a small function to write a digit into the fraction member, helpful because fraction_ contains two digits per byte
		dig_i++; //the first index is the sign of the
		fraction_[dig_i / 2] = (dig_i % 2) ?
			((dig & B00001111) | (fraction_[dig_i / 2] & B11110000)) :
			((dig << 4) | (fraction_[dig_i / 2] & B00001111));
	}
	utiny readFromFraction_(int dig_i) const {
		dig_i++; //the first index is the sign of the 
		return dig_i % 2 ?
			(fraction_[dig_i / 2] & B00001111) :
			(fraction_[dig_i / 2] >> 4);
	}
	void wtf(int dig_i, utiny dig) {
		writeToFraction_(dig_i, dig);
	}
	utiny rff(int dig_i) const {
		return readFromFraction_(dig_i);
	}
	void pos_add(Decimal_32 a, Decimal_32 b); //unsafely adds to numbers assuming they aren't signed and the mantissi are already equal, then sets object to it
	void pos_subtract(Decimal_32 a, Decimal_32 b); //this also assumes that a is bigger than b
public:
	static const int DIGITS_ = 61; //Max digits that Decimal_32 can store. Just good to own this constant if need to change later
	static const int DIGITBYTES_ = 31;
	~Decimal_32();
	Decimal_32();
	Decimal_32(float d, int digits);
	Decimal_32(double d, int digits);
	Decimal_32(std::initializer_list<utiny> fraction, utiny mantissa, bool signd); //remove this constructor once we have a function double or float constructor
	Decimal_32(const Decimal_32& d);
	void display(void) const;
	bool isSigned(void) const {
		return (fraction_[0] >> 4);
	}
	int digits(void) const {
		int space = 0;
		for (int i = 0; i < DIGITS_; i++) {//for (int i = DIGITS_ - 1; i >= 0; i--) {
			auto debuggr = rff(i);
			if (rff(i)) {
				break;
			}
			else {
				space++;
			}
		}
		return DIGITS_ - space;
	}
	void negate() {
		fraction_[0] = (!(fraction_[0] >> 4) << 4) | fraction_[0] & B00001111;
	}
	Decimal_32 abs(void) const;
	//Decimal_32 operator*(Decimal_32 b) const;
	friend Decimal_32 operator+(Decimal_32 a, Decimal_32 b); //TO DO: look into changing these to pass by reference and profile/benchmark; also pos_ privates, and other operators
	friend Decimal_32 operator-(Decimal_32 a, Decimal_32 b);
	friend bool operator<(Decimal_32 a, Decimal_32 b);
	friend bool operator>(Decimal_32 a, Decimal_32 b);
	//friend bool operator==(Decimal_32 a, Decimal_32 b);
	//friend bool operator<=(Decimal_32 a, Decimal_32 b);
	//friend bool operator>=(Decimal_32 a, Decimal_32 b);
	//Decimal_32 operator/(Decimal_32 b) const;
	Decimal_32& operator|(Decimal_32& b); //This operator just converts the two numbers to have equal mantissas //still returns a decimal_32& in case someone wants to inline it i guess
	Decimal_32 operator-() const;
};