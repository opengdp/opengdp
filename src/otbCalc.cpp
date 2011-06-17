

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif


#include <stdio.h>

#include "otbBandMathImageFilter.h"


#include "itkExceptionObject.h"
#include "otbImage.h"
#include "otbImageList.h"
//#include "otbImageToImageListFilter.h"
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "otbVectorRescaleIntensityImageFilter.h"
#include "otbMultiChannelExtractROI.h"



typedef double                                      PixelType; 
typedef otb::VectorImage<PixelType, 2>              InputImageType; 
typedef otb::Image<PixelType, 2>                    OutputImageType; 
typedef otb::ImageList<OutputImageType>             ImageListType; 
typedef OutputImageType::PixelType                  VPixelType; 
typedef otb::VectorImageToImageListFilter<InputImageType,ImageListType> 
                                                    VectorImageToImageListType; 
typedef otb::ImageFileReader<InputImageType>        ReaderType; 
typedef otb::ImageFileWriter<OutputImageType>       WriterType;


typedef otb::BandMathImageFilter<OutputImageType>   FilterType;

int main (int argc, char *argv[])
{

    ReaderType::Pointer reader = ReaderType::New(); 
    WriterType::Pointer writer = WriterType::New(); 

    FilterType::Pointer filter = FilterType::New(); 

    writer->SetInput(filter->GetOutput()); 
    reader->SetFileName(argv[1]); 
    writer->SetFileName(argv[2]);

    /***** We need now to extract now each band from the input   *****/
    /***** otb::VectorImage, it illustrates the use of the       *****/
    /***** otb::VectorImageToImageList. Each extracted layer are *****/
    /***** inputs of the otb::BandMathImageFilter:               *****/

    VectorImageToImageListType::Pointer imageList = VectorImageToImageListType::New(); 
    imageList->SetInput(reader->GetOutput()); 

    imageList->UpdateOutputInformation(); 

    const unsigned int nbBands = reader->GetOutput()->GetNumberOfComponentsPerPixel(); 

    for(unsigned int j = 0; j < nbBands; ++j) { 
        filter->SetNthInput(j, imageList->GetOutput()->GetNthElement(j)); 
    }

    filter->SetExpression(argv[3]);

    /***** We can now plug the pipeline and run it. *****/
    
    writer->Update();

    return EXIT_SUCCESS;
}