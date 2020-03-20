#ifndef TRAIT_H
#define TRAIT_H


#define DECL_TRAIT(TYPE, TRAIT)\
TRAIT * TYPE_##TRAIT( TYPE *self );

#define IMPL_TRAIT(TYPE, TRAIT)\
TRAIT * TYPE_##TRAIT( TYPE *self ) {\
  return &(self->_t_##TRAIT);\
}


#define ASTRAIT(PTR, TYPE, TRAIT) (&(((TYPE *)(PTR))->_i##TRAIT)) 

/**
 * Calculates offset of member inside the struct
 */
#define OOC_OFFSETOF(_struct_, _member_)  (size_t)&(((_struct_ *)0)->_member_)
 
/**
 * Calculates pointer to subclass (i.e. containing struct) from pointer to
 * superclass (i.e. contained struct).
 */
#define OOC_GET_CONTAINER_PT(                                              \
      _T_Subclass_,                                                        \
      _superclass_field_name_,                                             \
      _superclass_pt_                                                      \
      )                                                                    \
                                                                           \
   ((_T_Subclass_ *)(                                                      \
      (unsigned char *)(_superclass_pt_)                                   \
      - OOC_OFFSETOF(_T_Subclass_, _superclass_field_name_)                \
      )                                                                    \
    )


#endif