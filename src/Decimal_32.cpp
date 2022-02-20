#include "Decimal_32.h"

Decimal_32::~Decimal_32() {
	//does nothing
}

Decimal_32::Decimal_32() {
	for (int i = 0; i < 31; i++) {
		fraction_[i] = B00000000;
	}
	mantissa_ = 158;
}

Decimal_32::Decimal_32(float d, int digits) {
	std::cout << "I will implement this function later\n";
}

Decimal_32::Decimal_32(double d, int digits) {
	std::cout << "I will implement this function later\n";
}

Decimal_32::Decimal_32(std::initializer_list<utiny> fraction, utiny mantissa, bool signd) {
	mantissa_ = mantissa;
	for (int i = 0; i < fraction.size(); i++) {
		wtf(i + DIGITS_ - fraction.size(), fraction.begin()[i]);
	}
	if ((DIGITS_ - fraction.size() - 1) % 2) {
		wtf(DIGITS_ - fraction.size() - 1, 0);
	}
	for (int i = (DIGITS_ - fraction.size() + 1/*Offset for sign*/ - 1/*it's an index*/ - 1/*We go one back*/) / 2; i >= 0; i--) {
		fraction_[i] = B00000000;
	}
	fraction_[0] = signd ? B00010000 : B00000000;
}

Decimal_32::Decimal_32(const Decimal_32& d) {
	for (int i = 0; i < DIGITBYTES_; i++) {
		fraction_[i] = d.fraction_[i]; //obv just copies bytes over
	}
	mantissa_ = d.mantissa_;
}

void Decimal_32::display(void) const {
	auto digitscache = digits();
	int dplocation = (int)mantissa_ - 158 + digitscache; 
	int dpiterlocation = DIGITS_ + dplocation - digitscache; 
	if (isSigned()) {
		std::cout << '-';
	}
	if (dplocation <= 0) {
		std::cout << '.';
		for (int _ = dplocation; _ <= 0; _++) { //use an underscore if the variable is not needed within the loop but just to keep track of it
			std::cout << '0';
		}
		for (int i = DIGITS_ - digitscache; i < DIGITS_; i++) {
			std::cout << (int)rff(i);
		}
	}
	else {
		for (int i = DIGITS_ - digitscache; i < DIGITS_; i++) { //more efficient alternative if there is an integrated decimal point
			if (dpiterlocation == i) {
				std::cout << '.';
			}
			std::cout << (int)rff(i);
		}
	}
}

Decimal_32 Decimal_32::abs(void) const {
	Decimal_32 ret = *this;
	ret.fraction_[0] = B00001111 & ret.fraction_[0]; //clear leading sign half-byte
	return ret;
}

Decimal_32& Decimal_32::operator|(Decimal_32& b) {
	//Count avaiable shifting slots for both numbers
	int spacea = 0;
	int spaceb = 0;
	for (int i = 1; i < /*a.*/DIGITS_; i++) {
		if (!(i % 2 ? /*a.*/fraction_[i / 2] & B00001111 : (/*a.*/fraction_[i / 2]) >> 4)) {
			spacea++;
		}
		else {
			break;
		}
	}
	for (int i = 1; i < b.DIGITS_/*Using B here because b is used in the rest of the function*/; i++) {
		if (!(i % 2 ? b.fraction_[i / 2] & B00001111 : (b.fraction_[i / 2]) >> 4)) {
			spaceb++;
		}
		else {
			break;
		}
	}
	bool adderbool = ((int)/*a.*/mantissa_ - spacea < (int)b.mantissa_ - spaceb); //inverted because lesser should be the adder; then it will offset both and add
	Decimal_32& adder = adderbool ? *this : b;
	Decimal_32& addee = adderbool ? b : *this;
	int mantissaDifference = diff(/*a.*/mantissa_, b.mantissa_);
	//offset the adder
	int addeeOffset = min(mantissaDifference, (!adderbool) ? spacea : spaceb);
	for (int i = 0; i < addee.DIGITS_ - addeeOffset; i++) { //goes through and shifts
		addee.wtf(i, addee.rff(i + addeeOffset));
	}
	for (int i = addee.DIGITS_ - addeeOffset; i < addee.DIGITS_; i++) { // flushes after data
		addee.wtf(i, 0); 
	}
	addee.mantissa_ -= addeeOffset; //TO DO consider max/min mantissi
	int adderOffset = mantissaDifference - addeeOffset;
	if (adderOffset) {
		for (int i = adder.DIGITS_ - 1; i >= adderOffset; i--) { //goes through and shifts
			adder.wtf(i, adder.rff(i - adderOffset));
		}
		for (int i = adderOffset - 1; i >= 0; i--) { // flushes after data
			adder.wtf(i, 0); 
		}
	}
	adder.mantissa_ += adderOffset; //TO DO consider max/min mantissi
	return adder;
}

Decimal_32 operator+(Decimal_32 a, Decimal_32 b) {
	bool signa = a.fraction_[0] >> 4;
	bool signb = b.fraction_[0] >> 4;
	if (!signa && signb) return a - b.abs();
	if (signa && !signb) return b - a.abs();
	if (signa && signb) return -(a.abs() + b.abs());
	Decimal_32 c;
	utiny carry = 0;

	a | b; // TO DO: use the output of the space search to quit adding to save time
	c.mantissa_ = a.mantissa_;

	for (int i = c.DIGITS_ - 1; i >= 0; i--) {
		/*utiny a_currentDigit = i % 2 ? a.fraction_[i / 2] & B00001111 : (a.fraction_[i / 2]) >> 4;
		utiny b_currentDigit = i % 2 ? b.fraction_[i / 2] & B00001111 : (b.fraction_[i / 2]) >> 4;
		utiny c_currentDigit = a_currentDigit + b_currentDigit + carry; // old code*/
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit + b_currentDigit + carry;
		if (c_currentDigit > 9/*Max number in decimal*/) {
			carry = 1;
			c_currentDigit = 9;
		}
		else {
			carry = 0; //clear carried number
		}
		c.wtf(i, c_currentDigit); 
	}
	if (carry == 1) { //this means the number is too high for the current mantissa, need to scootch everything over
		/*for (int i = Decimal_32::DIGITBYTES_ - 1; i >= 1; i--) {
			c.fraction_[i] = c.fraction_[i] >> 4 | (c.fraction_[i - 1] << 4);
		}
		c.fraction_[0] = (c.fraction_[0] & B11110000) | B00000001; /*adds in rolled over 1*/
		for (int i = c.DIGITS_ - 1; i >= 0 + 1/*because it is scootching it shouldn't go past 1*/; i--) {
			c.wtf(i, c.rff(i - 1));
		}
		c.wtf(0, 1);
		if (c.mantissa_ < 255) c.mantissa_++; //TO DO how should it handle if the mantissa is already maxed
	}
	return c;
}

Decimal_32 operator-(Decimal_32 a, Decimal_32 b) {
	std::cout << "I will implement this function later\n";
	return a;
}

Decimal_32 Decimal_32::/*Is not a friend function, stays in scope*/operator-() const {
	Decimal_32 ret = *this;
	ret.fraction_[0] = ((ret.fraction_[0] ^ B11111111) & B11110000) | ret.fraction_[0]; //clear leading sign half-byte
	return ret;
}