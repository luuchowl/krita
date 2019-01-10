#ifndef LCMSRGBP2020PQCOLORSPACE_H
#define LCMSRGBP2020PQCOLORSPACE_H

#include <colorspaces/rgb_u8/RgbU8ColorSpace.h>
#include <colorspaces/rgb_u16/RgbU16ColorSpace.h>
#include <colorspaces/rgb_f16/RgbF16ColorSpace.h>
#include <colorspaces/rgb_f32/RgbF32ColorSpace.h>

#include "KoColorConversionTransformationFactory.h"

#include <LcmsRGBP2020PQColorSpaceTransformation.h>

template <class T>
struct ColorSpaceFromFactory {
};

template<>
struct ColorSpaceFromFactory<RgbU8ColorSpaceFactory> {
  typedef RgbU8ColorSpace type;
};

template<>
struct ColorSpaceFromFactory<RgbU16ColorSpaceFactory> {
  typedef RgbU16ColorSpace type;
};

template<>
struct ColorSpaceFromFactory<RgbF16ColorSpaceFactory> {
  typedef RgbF16ColorSpace type;
};

template<>
struct ColorSpaceFromFactory<RgbF32ColorSpaceFactory> {
  typedef RgbF32ColorSpace type;
};

/**
 *  Define a singly linked list of supported bit depth traits
 */
template<class T> struct NextTrait { using type = void; };
template<> struct NextTrait<KoBgrU8Traits> { using type = KoBgrU16Traits; };
template<> struct NextTrait<KoBgrU16Traits> { using type = KoRgbF16Traits; };
template<> struct NextTrait<KoRgbF16Traits> { using type = KoRgbF32Traits; };

/**
 * Recursively add bit-depths conversions to the color space. We add only
 * **outgoing** conversions for every RGB color space. That is, every color
 * space has exactly three outgoing edges for color conversion.
 */
template<typename ParentColorSpace, typename CurrentTraits>
void addInternalConversion(QList<KoColorConversionTransformationFactory*> &list, CurrentTraits*)
{
    // general case: add a converter and recurse for the next traits
    list << new LcmsScaleRGBP2020PQTransformationFactory<ParentColorSpace, CurrentTraits>();

    using NextTraits = typename NextTrait<CurrentTraits>::type;
    addInternalConversion<ParentColorSpace>(list, static_cast<NextTraits*>(0));
}

template<typename ParentColorSpace>
void addInternalConversion(QList<KoColorConversionTransformationFactory*> &list, typename ParentColorSpace::ColorSpaceTraits*)
{
    // exception: skip adding an edge to the same bit depth

    using CurrentTraits = typename ParentColorSpace::ColorSpaceTraits;
    using NextTraits = typename NextTrait<CurrentTraits>::type;
    addInternalConversion<ParentColorSpace>(list, static_cast<NextTraits*>(0));
}

template<typename ParentColorSpace>
void addInternalConversion(QList<KoColorConversionTransformationFactory*> &, void*)
{
    // stop recursion
}

template <class BaseColorSpaceFactory>
class LcmsRGBP2020PQColorSpaceFactoryWrapper : public BaseColorSpaceFactory
{
    typedef typename ColorSpaceFromFactory<BaseColorSpaceFactory>::type RelatedColorSpaceType;

    KoColorSpace *createColorSpace(const KoColorProfile *p) const override
    {
        return new RelatedColorSpaceType(this->name(), p->clone());
    }

    QList<KoColorConversionTransformationFactory *> colorConversionLinks() const override
    {
        QList<KoColorConversionTransformationFactory *> list;

        // we skip direct conversions to RGB U8, because it cannot fit linear color space
        list << new LcmsFromRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoBgrU16Traits>();
        list << new LcmsFromRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoRgbF16Traits>();
        list << new LcmsFromRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoRgbF32Traits>();
        list << new LcmsToRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoBgrU16Traits>();
        list << new LcmsToRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoRgbF16Traits>();
        list << new LcmsToRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoRgbF32Traits>();

        // internally, we can convert to RGB U8 if needed
        addInternalConversion<RelatedColorSpaceType>(list, static_cast<KoBgrU8Traits*>(0));

        return list;
    }
};

#endif // LCMSRGBP2020PQCOLORSPACE_H
