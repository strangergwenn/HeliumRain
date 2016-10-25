#pragma once

#include <EditorStyle.h>
#include <PropertyEditing.h>

/**
* Customizes an InputActionKeyMapping struct to display it more simply.
* Modified to have wider action selection fields.
*/
class FInputActionMappingCustomizationExtended : public IPropertyTypeCustomization
{
public:
	static bool Register();
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
private:
	void RemoveActionMappingButton_OnClick();
private:
	TSharedPtr<class IPropertyHandle> ActionMappingHandle;
};
/**
* Customizes an InputAxisKeyMapping struct to display it more simply.
* Modified to have wider axis selection fields.
*/
class FInputAxisMappingCustomizationExtended : public IPropertyTypeCustomization
{
public:
	static bool Register();
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
private:
	void RemoveAxisMappingButton_OnClick();
private:
	TSharedPtr<class IPropertyHandle> AxisMappingHandle;
};

