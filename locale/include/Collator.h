#ifndef _COLLATOR_H_
#define _COLLATOR_H_


#include <SupportDefs.h>


class BString;
class BCollatorAddOn;


enum collator_strengths {
	B_COLLATE_DEFAULT = -1,

	B_COLLATE_PRIMARY = 1,		// e.g.: no diacritical differences, e = é
	B_COLLATE_SECONDARY,		// diacritics are different from their base characters, a != ä
	B_COLLATE_TERTIARY,			// case sensitive comparison
	B_COLLATE_QUATERNARY,

	B_COLLATE_IDENTICAL = 127	// Unicode value
};


class BCollator {
	public:
		BCollator();
		~BCollator();

		void SetDefaultStrength(int8 strength);
		int8 DefaultStrength() const;

		void SetIgnorePunctuation(bool ignore);
		bool IgnorePunctuation() const;

		void GetSortKey(const char *string, BString *key, int8 strength = B_COLLATE_DEFAULT);

		int Compare(const char *, const char *, int32 len = -1, int8 strength = B_COLLATE_DEFAULT);
		bool Equal(const char *, const char *, int32 len = -1, int8 strength = B_COLLATE_DEFAULT);
		bool Greater(const char *, const char *, int32 len = -1, int8 strength = B_COLLATE_DEFAULT);
		bool GreaterOrEqual(const char *, const char *, int32 len = -1, int8 strength = B_COLLATE_DEFAULT);

	private:
		BCollatorAddOn	*fCollator;
		int8			fStrength;
		bool			fIgnorePunctuation;
};


inline bool 
BCollator::Equal(const char *s1, const char *s2, int32 len, int8 strength)
{
	return Compare(s1, s2, len, strength) == 0;
}


inline bool 
BCollator::Greater(const char *s1, const char *s2, int32 len, int8 strength)
{
	return Compare(s1, s2, len, strength) > 0;
}


inline bool 
BCollator::GreaterOrEqual(const char *s1, const char *s2, int32 len, int8 strength)
{
	return Compare(s1, s2, len, strength) >= 0;
}


/************************************************************************/

// For BCollator add-on implementations:

class BCollatorAddOn {
	public:
		BCollatorAddOn();
		virtual ~BCollatorAddOn();

		virtual void GetSortKey(const char *string, BString *key, int8 strength, bool ignorePunctuation);
		virtual int Compare(const char *a, const char *b, int32 length, int8 strength, bool ignorePunctuation);
};

#endif	/* _COLLATOR_H_ */
