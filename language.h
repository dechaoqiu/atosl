/* Languages represented in the symbol table and elsewhere.
 */

enum language
  {
    language_unknown,		/* Language not known */
    language_auto,		/* Placeholder for automatic setting */
    language_c,			/* C */
    language_cplus,		/* C++ */
    language_objc,		/* Objective-C */
    /* APPLE LOCAL objcplus */
    language_objcplus,		/* Objective-C++ */
    language_java,		/* Java */
    language_fortran,		/* Fortran */
    language_m2,		/* Modula-2 */
    language_asm,		/* Assembly language */
    language_scm,    		/* Scheme / Guile */
    language_pascal,		/* Pascal */
    language_ada,		/* Ada */
    language_minimal,		/* All other languages, minimal support only */
    nr_languages
  };

