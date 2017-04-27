#if WITH_EDITOR

#include "InputSettingsCustomization.h"
#include <Engine.h>

#include <Editor.h>
#include <DetailCustomizations.h>
#include <PropertyCustomizationHelpers.h>
#include <PropertyEditorDelegates.h>

#define LOCTEXT_NAMESPACE "InputStructCustomization"

namespace InputConstants
{
	const FMargin PropertyPadding(2.0f, 0.0f, 2.0f, 0.0f);
	const float TextBoxWidth = 200.0f;
	const float ScaleBoxWidth = 50.0f;
}

// For 4.6 compatibility
struct TextOrStringConverter
{
	FText text;
        TextOrStringConverter(const FText& Text) : text(Text) {}
	operator FText() { return text; }
	operator FString() { return text.ToString(); }
};

bool FInputActionMappingCustomizationExtended::Register()
{
	FModuleManager::LoadModuleChecked<FDetailCustomizationsModule>("DetailCustomizations");

	FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate = FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInputActionMappingCustomizationExtended::MakeInstance);
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout("InputActionKeyMapping");
	PropertyModule.RegisterCustomPropertyTypeLayout("InputActionKeyMapping", PropertyTypeLayoutDelegate);
	PropertyModule.NotifyCustomizationModuleChanged();

	return true;
}

bool FInputAxisMappingCustomizationExtended::Register()
{
	FModuleManager::LoadModuleChecked<FDetailCustomizationsModule>("DetailCustomizations");

	FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate = FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInputAxisMappingCustomizationExtended::MakeInstance);
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout("InputAxisKeyMapping");
	PropertyModule.RegisterCustomPropertyTypeLayout("InputAxisKeyMapping", PropertyTypeLayoutDelegate);
	PropertyModule.NotifyCustomizationModuleChanged();

	return true;
}

//////////////////////////////////////
// FInputActionMappingCustomization //
//////////////////////////////////////

TSharedRef<IPropertyTypeCustomization> FInputActionMappingCustomizationExtended::MakeInstance()
{
	return MakeShareable(new FInputActionMappingCustomizationExtended);
}
void FInputActionMappingCustomizationExtended::CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	ActionMappingHandle = InStructPropertyHandle;
}
void FInputActionMappingCustomizationExtended::CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> KeyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, Key));
	TSharedPtr<IPropertyHandle> ShiftHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, bShift));
	TSharedPtr<IPropertyHandle> CtrlHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, bCtrl));
	TSharedPtr<IPropertyHandle> AltHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, bAlt));
	TSharedPtr<IPropertyHandle> CmdHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, bCmd));
	TSharedRef<SWidget> RemoveButton = PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &FInputActionMappingCustomizationExtended::RemoveActionMappingButton_OnClick),
		LOCTEXT("RemoveActionMappingToolTip", "Removes Action Mapping"));
	StructBuilder.AddChildContent(TextOrStringConverter(LOCTEXT("KeySearchStr", "Key")))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(InputConstants::PropertyPadding)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(InputConstants::TextBoxWidth*2.0f)
				[
					StructBuilder.GenerateStructValueWidget(KeyHandle.ToSharedRef())
				]
			]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					ShiftHandle->CreatePropertyNameWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					ShiftHandle->CreatePropertyValueWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					CtrlHandle->CreatePropertyNameWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					CtrlHandle->CreatePropertyValueWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					AltHandle->CreatePropertyNameWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					AltHandle->CreatePropertyValueWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					CmdHandle->CreatePropertyNameWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					CmdHandle->CreatePropertyValueWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					RemoveButton
				]
		];
}
void FInputActionMappingCustomizationExtended::RemoveActionMappingButton_OnClick()
{
	if (ActionMappingHandle->IsValidHandle())
	{
		const TSharedPtr<IPropertyHandle> ParentHandle = ActionMappingHandle->GetParentHandle();
		const TSharedPtr<IPropertyHandleArray> ParentArrayHandle = ParentHandle->AsArray();
		ParentArrayHandle->DeleteItem(ActionMappingHandle->GetIndexInArray());
	}
}

//////////////////////////////////////
// FInputAxisMappingCustomization //
//////////////////////////////////////

TSharedRef<IPropertyTypeCustomization>FInputAxisMappingCustomizationExtended::MakeInstance()
{
	return MakeShareable(new FInputAxisMappingCustomizationExtended);
}

void FInputAxisMappingCustomizationExtended::CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	AxisMappingHandle = InStructPropertyHandle;
}

void FInputAxisMappingCustomizationExtended::CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> KeyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputAxisKeyMapping, Key));
	TSharedPtr<IPropertyHandle> ScaleHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputAxisKeyMapping, Scale));
	TSharedRef<SWidget> RemoveButton = PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &FInputAxisMappingCustomizationExtended::RemoveAxisMappingButton_OnClick),
		LOCTEXT("RemoveAxisMappingToolTip", "Removes Axis Mapping"));
	StructBuilder.AddChildContent(TextOrStringConverter(LOCTEXT("KeySearchStr", "Key")))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(InputConstants::PropertyPadding)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(InputConstants::TextBoxWidth * 2.0f)
				[
					StructBuilder.GenerateStructValueWidget(KeyHandle.ToSharedRef())
				]
			]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					ScaleHandle->CreatePropertyNameWidget()
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(InputConstants::ScaleBoxWidth)
					[
						ScaleHandle->CreatePropertyValueWidget()
					]
				]
			+ SHorizontalBox::Slot()
				.Padding(InputConstants::PropertyPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					RemoveButton
				]
		];
}

void FInputAxisMappingCustomizationExtended::RemoveAxisMappingButton_OnClick()
{
	if (AxisMappingHandle->IsValidHandle())
	{
		const TSharedPtr<IPropertyHandle> ParentHandle = AxisMappingHandle->GetParentHandle();
		const TSharedPtr<IPropertyHandleArray> ParentArrayHandle = ParentHandle->AsArray();
		ParentArrayHandle->DeleteItem(AxisMappingHandle->GetIndexInArray());
	}
}

#undef LOCTEXT_NAMESPACE

#endif
