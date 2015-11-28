/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkLabelImageToLabelMapFilter.h"
#include "itkLabelMapToLabelImageFilter.h"
#include "itkLabelSelectionLabelMapFilter.h"
#include "itkMergeLabelMapFilter.h"
#include "itkObjectByObjectLabelMapFilter.h"
#include "itkLabelUniqueLabelMapFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"

int main( int argc, char* argv[] )
{
  if( argc != 5 )
    {
    std::cerr << "Usage: "<< std::endl;
    std::cerr << argv[0];
    std::cerr << " <InputFileName> <OutputFileName> <label> <radius>";
    std::cerr << std::endl;
    return EXIT_FAILURE;
    }

  const unsigned int Dimension = 2;

  typedef unsigned char                      PixelType;
  typedef itk::Image< PixelType, Dimension > ImageType;

  const char * inputFileName = argv[1];
  const char * outputFileName = argv[2];
  const PixelType label = static_cast< PixelType >( atoi( argv[3] ) );
  const unsigned int radiusValue = atoi( argv[4] );

  typedef itk::ImageFileReader< ImageType >  ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputFileName );

  typedef itk::LabelObject< PixelType, Dimension >  LabelObjectType;
  typedef itk::LabelMap< LabelObjectType >          LabelMapType;

  typedef itk::LabelImageToLabelMapFilter< ImageType, LabelMapType > LabelImageToLabelMapFilterType;
  LabelImageToLabelMapFilterType::Pointer labelMapConverter = LabelImageToLabelMapFilterType::New();
  labelMapConverter->SetInput( reader->GetOutput() );
  labelMapConverter->SetBackgroundValue( itk::NumericTraits< PixelType >::Zero );

  typedef itk::LabelSelectionLabelMapFilter< LabelMapType > SelectorType;
  SelectorType::Pointer selector = SelectorType::New();
  selector->SetInput( labelMapConverter->GetOutput() );
  selector->SetLabel( label );

  typedef itk::FlatStructuringElement< Dimension > StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill( radiusValue );

  StructuringElementType structuringElement = StructuringElementType::Ball( radius );

  typedef itk::BinaryMorphologicalClosingImageFilter< ImageType, ImageType, StructuringElementType > MorphologicalFilterType;
  MorphologicalFilterType::Pointer closingFilter = MorphologicalFilterType::New();

  typedef itk::ObjectByObjectLabelMapFilter< LabelMapType > ObjectByObjectLabelMapFilterType;
  ObjectByObjectLabelMapFilterType::Pointer objectByObjectLabelMapFilter = ObjectByObjectLabelMapFilterType::New();
  objectByObjectLabelMapFilter->SetInput( selector->GetOutput( 0 ) );
  objectByObjectLabelMapFilter->SetBinaryInternalOutput( true );
  objectByObjectLabelMapFilter->SetFilter( closingFilter );

  typedef itk::MergeLabelMapFilter< LabelMapType > MergeLabelFilterType;
  MergeLabelFilterType::Pointer merger = MergeLabelFilterType::New();
  merger->SetInput( 0, objectByObjectLabelMapFilter->GetOutput( 0 ) );
  merger->SetInput( 1, selector->GetOutput( 1 ) );
  merger->SetMethod( MergeLabelFilterType::KEEP );

  typedef itk::LabelUniqueLabelMapFilter< LabelMapType > UniqueLabelMapFilterType;
  UniqueLabelMapFilterType::Pointer unique = UniqueLabelMapFilterType::New();
  unique->SetInput( merger->GetOutput() );

  typedef itk::LabelMapToLabelImageFilter< LabelMapType, ImageType > LabelMapToLabelImageFilterType;
  LabelMapToLabelImageFilterType::Pointer labelImageConverter = LabelMapToLabelImageFilterType::New();
  labelImageConverter->SetInput( unique->GetOutput() );

  typedef itk::ImageFileWriter< ImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputFileName );
  writer->SetInput( labelImageConverter->GetOutput() );
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & error )
    {
    std::cerr << "Error: " << error << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
