#ifndef _NL_TYPES_H_
#define _NL_TYPES_H_

#define	NL_SETD			0
#define	NL_CAT_LOCALE	1

typedef	int nl_item;
typedef	void *nl_catd;

#include <LocaleBuild.h>

#ifdef __cplusplus
extern "C" {
#endif

extern _IMPEXP_LOCALE nl_catd catopen(const char *name, int oflag);
extern _IMPEXP_LOCALE char *catgets(nl_catd cat,int setID,int msgID,const char *defaultMessage);
extern _IMPEXP_LOCALE int catclose(nl_catd cat);

#ifdef __cplusplus
}
#endif

#endif	/* _NL_TYPES_H_ */
