/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "Simple"
 * 	found in "/home/styler/git/fast_ber/testfiles/simple5.asn"
 * 	`asn1c -S /home/styler/git/fast_ber/3rd_party/asn1c/skeletons -fno-constraints -fno-include-deps`
 */

#ifndef	_Collection_H_
#define	_Collection_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <NativeInteger.h>
#include <BOOLEAN.h>
#include "Child.h"
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum the_choice_PR {
	the_choice_PR_NOTHING,	/* No components present */
	the_choice_PR_hello,
	the_choice_PR_goodbye,
	the_choice_PR_integer,
	the_choice_PR_boolean,
	/* Extensions may appear below */
	
} the_choice_PR;

/* Forward declarations */
struct Child;

/* Collection */
typedef struct Collection {
	OCTET_STRING_t	 hello;
	OCTET_STRING_t	 goodbye;
	long	 integer;
	BOOLEAN_t	 boolean;
	Child_t	 child;
	struct Child	*optional_child	/* OPTIONAL */;
	struct the_choice {
		the_choice_PR present;
		union Collection__the_choice_u {
			OCTET_STRING_t	 hello;
			OCTET_STRING_t	 goodbye;
			long	 integer;
			BOOLEAN_t	 boolean;
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} the_choice;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Collection_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Collection;

#ifdef __cplusplus
}
#endif

#endif	/* _Collection_H_ */
#include <asn_internal.h>