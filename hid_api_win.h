/******************************************************************************
 * Name:      hid_api_win.h
 * Purpose:   wxThread based USB HID API
 * Author:    Ingolf Tippner (info@tecno-world.com)
 * Created:   2017-08-08
 * Copyright (C) 2017  Ingolf Tippner (Tecno-World, Pitius Tec S.L.)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.

 * Changelog:

********************************************************************************/

#ifndef HID_API_WIN_H
#define HID_API_WIN_H

#include <wchar.h>
#include <vector>
#include "wx/thread.h"

/* Copied from inc/ddk/hidclass.h, part of the Windows DDK.*/
#define HID_OUT_CTL_CODE(id)  \
    CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_HID_GET_FEATURE                   HID_OUT_CTL_CODE(100)




class HID_API : public wxThread
{
    public:
        typedef void (*PollingEvent_t)(unsigned char* buffer, int cnt);
        HID_API(PollingEvent_t EvtFunc);
        virtual ~HID_API();
        void* Entry();

		struct hid_device_info_t
		{
			wxString m_path;
			unsigned short vendor_id;
			unsigned short product_id;
			unsigned short release_number;
			wxString serial_number;
			wxString manufacturer_string;
			wxString product_string;
			unsigned short usage_page;
			unsigned short usage;
			int interface_number;
		};

		bool isInitialized(){return is_enumerated;};
		//returns the index of the device, -1 if the no next device.
		int GetDeviceCount(){return device_list.size();};
        int GetFirstDevice(hid_device_info_t &Info);
        int GetNextDevice(hid_device_info_t &Info);
        bool OpenDevice(const char *path);
        void CloseDevice();
		void EnablePolling(bool enable);
		bool GetInputReport(char* buffer, size_t len);
		const wxString LastError(){return hid_error(m_device);};

    private:
        PollingEvent_t PollEvt;
        bool IntReadEnabled;
		int  hid_init(void);
        struct hid_device
        {
            HANDLE device_handle;
            BOOL blocking;
            USHORT output_report_length;
            size_t input_report_length;
            wxString last_error_str;
            DWORD last_error_num;
            BOOL read_pending;
            char read_buf[256];
            char out_buf[256];
            char feat_buf[256];
            OVERLAPPED ol;
            void Clear();
        };

        struct HIDD_ATTRIBUTES_t
        {
            ULONG Size;
            USHORT VendorID;
            USHORT ProductID;
            USHORT VersionNumber;
        };
        //_HIDD_ATTRIBUTES *PHIDD_ATTRIBUTES;

        struct HIDP_CAPS_t
        {
            USHORT Usage;
            USHORT UsagePage;
            USHORT InputReportByteLength;
            USHORT OutputReportByteLength;
            USHORT FeatureReportByteLength;
            USHORT Reserved[17];
            USHORT fields_not_used_by_hidapi[10];
        };

        typedef void* PHIDP_PREPARSED_DATA;
        #define HIDP_STATUS_SUCCESS 0x110000
        typedef bool (__stdcall *HidD_GetAttributes_)(HANDLE device, HIDD_ATTRIBUTES_t* attrib);
        typedef bool (__stdcall *HidD_GetSerialNumberString_)(HANDLE device, PVOID buffer, ULONG buffer_len);
        typedef bool (__stdcall *HidD_GetManufacturerString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
        typedef bool (__stdcall *HidD_GetProductString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
        typedef bool (__stdcall *HidD_SetFeature_)(HANDLE handle, PVOID data, ULONG length);
        typedef bool (__stdcall *HidD_GetFeature_)(HANDLE handle, PVOID data, ULONG length);
        typedef bool (__stdcall *HidD_GetIndexedString_)(HANDLE handle, ULONG string_index, PVOID buffer, ULONG buffer_len);
        typedef bool (__stdcall *HidD_GetPreparsedData_)(HANDLE handle, PHIDP_PREPARSED_DATA *preparsed_data);
        typedef bool (__stdcall *HidD_FreePreparsedData_)(PHIDP_PREPARSED_DATA preparsed_data);
        typedef bool (__stdcall *HidD_SetNumInputBuffers_)(HANDLE handle, ULONG number_buffers);

        typedef long (__stdcall *HidP_GetCaps_)(PHIDP_PREPARSED_DATA preparsed_data, HIDP_CAPS_t* caps);
        //neu...
        typedef bool (__stdcall *HidD_GetInputReport_) (HANDLE HidDeviceObject, PVOID ReportBuffer, ULONG ReportBufferLength);
        typedef bool (__stdcall *HidD_SetOutputReport_) (HANDLE HidDeviceObject, PVOID  ReportBuffer, ULONG  ReportBufferLength);

#ifdef __COMPLETE__
        typedef bool (__stdcall *HidD_GetHidGuid_) ();
        typedef bool (__stdcall *HidD_GetPhysicalDescriptor_) ();
        typedef bool (__stdcall *HidD_GetNumInputBuffers_) ();
        typedef bool (__stdcall *HidP_GetButtonCaps_) ();
        typedef bool (__stdcall *HidP_GetData_) ();
        typedef bool (__stdcall *HidP_GetExtendedAttributes_) ();
        typedef bool (__stdcall *HidP_GetLinkCollectionNodes_) ();
        typedef bool (__stdcall *HidP_GetScaledUsageValue_) ();
        typedef bool (__stdcall *HidP_GetSpecificButtonCaps_) ();
        typedef bool (__stdcall *HidP_GetSpecificValueCaps_) ();
        typedef bool (__stdcall *HidP_GetUsages_) ();
        typedef bool (__stdcall *HidP_GetUsagesEx_) ();
        typedef bool (__stdcall *HidP_GetUsageValue_) ();
        typedef bool (__stdcall *HidP_GetUsageValueArray_) ();
        typedef bool (__stdcall *HidP_GetValueCaps_) ();
        typedef bool (__stdcall *HidP_InitializeReportForID_) ();
        typedef bool (__stdcall *HidP_MaxDataListLength_) ();
        typedef bool (__stdcall *HidP_MaxUsageListLength_) ();
        typedef bool (__stdcall *HidP_SetData_) ();
        typedef bool (__stdcall *HidP_SetScaledUsageValue_) ();
        typedef bool (__stdcall *HidP_SetUsages_) ();
        typedef bool (__stdcall *HidP_SetUsageValue_) ();
        typedef bool (__stdcall *HidP_SetUsageValueArray_) ();
        typedef bool (__stdcall *HidP_UnsetUsages_) ();
        typedef bool (__stdcall *HidP_UsageListDifference_) ();

        typedef bool (__stdcall * HidD_FlushQueue_ ) ();
        typedef bool (__stdcall * HidD_GetConfiguration_ ) ();
        typedef bool (__stdcall * HidD_GetMsGenreDescriptor_ ) ();
        typedef bool (__stdcall * HidD_Hello_ ) ();
        typedef bool (__stdcall * HidD_SetConfiguration_ ) ();
#endif // __COMPLETE__

        static HidD_GetAttributes_ HidD_GetAttributes;
        static HidD_GetSerialNumberString_ HidD_GetSerialNumberString;
        static HidD_GetManufacturerString_ HidD_GetManufacturerString;
        static HidD_GetProductString_ HidD_GetProductString;
        static HidD_SetFeature_ HidD_SetFeature;
        static HidD_GetFeature_ HidD_GetFeature;
        static HidD_GetIndexedString_ HidD_GetIndexedString;
        static HidD_GetPreparsedData_ HidD_GetPreparsedData;
        static HidD_FreePreparsedData_ HidD_FreePreparsedData;
        static HidD_SetNumInputBuffers_ HidD_SetNumInputBuffers;

        static HidP_GetCaps_ HidP_GetCaps;
        //neu..
        static HidD_GetInputReport_ HidD_GetInputReport;
        static HidD_SetOutputReport_ HidD_SetOutputReport;

#ifdef __COMPLETE__
        static HidP_GetButtonCaps_ HidP_GetButtonCaps;
        static HidP_GetData_ HidP_GetData;
        static HidP_GetExtendedAttributes_ HidP_GetExtendedAttributes;
        static HidP_GetLinkCollectionNodes_ HidP_GetLinkCollectionNodes;
        static HidP_GetScaledUsageValue_ HidP_GetScaledUsageValue;
        static HidP_GetSpecificButtonCaps_ HidP_GetSpecificButtonCaps;
        static HidP_GetSpecificValueCaps_ HidP_GetSpecificValueCaps;
        static HidP_GetUsages_ HidP_GetUsages;
        static HidP_GetUsagesEx_ HidP_GetUsagesEx;
        static HidP_GetUsageValue_ HidP_GetUsageValue;
        static HidP_GetUsageValueArray_ HidP_GetUsageValueArray;
        static HidP_GetValueCaps_ HidP_GetValueCaps;
        static HidP_InitializeReportForID_ HidP_InitializeReportForID;
        static HidP_MaxDataListLength_ HidP_MaxDataListLength;
        static HidP_MaxUsageListLength_ HidP_MaxUsageListLength;
        static HidP_SetData_ HidP_SetData;
        static HidP_SetScaledUsageValue_ HidP_SetScaledUsageValue;
        static HidP_SetUsages_ HidP_SetUsages;
        static HidP_SetUsageValue_ HidP_SetUsageValue;
        static HidP_SetUsageValueArray_ HidP_SetUsageValueArray;
        static HidP_UnsetUsages_ HidP_UnsetUsages;
        static HidP_UsageListDifference_ HidP_UsageListDifference;

        static HidD_GetHidGuid_ HidD_GetHidGuid;
        static HidD_GetPhysicalDescriptor_ HidD_GetPhysicalDescriptor;
        static HidD_GetNumInputBuffers_ HidD_GetNumInputBuffers;
        static HidD_FlushQueue_ HidD_FlushQueue;
        static HidD_GetConfiguration_ HidD_GetConfiguration;
        static HidD_GetMsGenreDescriptor_ HidD_GetMsGenreDescriptor;
        static HidD_Hello_ HidD_Hello;
        static HidD_SetConfiguration_ HidD_SetConfiguration;
#endif // __COMPLETE__

        static HMODULE lib_handle;
        static BOOLEAN initialized;
        bool is_enumerated;
		int  hid_enumerate(unsigned short vendor_id=0, unsigned short product_id=0);

        static hid_device new_hid_device;
        static void free_hid_device(hid_device *dev);
        static void register_error(hid_device *device, wxString op);
        static int lookup_functions();
        static HANDLE open_device(const char *path, BOOL enumerate);
        std::vector<hid_device_info_t> device_list;
        static hid_device* m_device;
        static bool adv_install;
        int NextDeviceIdx;
		hid_device*  hid_open_path(const char *path);
		void hid_close(hid_device *device);

    protected:
		const wxString  hid_error(hid_device *device);
		int  hid_write(hid_device *device, const unsigned char *data, size_t length);
		int  hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds);
		int  hid_read(hid_device *device, unsigned char *data, size_t length);
		int  hid_set_nonblocking(hid_device *device, int nonblock);
		int  hid_send_feature_report(hid_device *device, const unsigned char *data, size_t length);
		int  hid_get_feature_report(hid_device *device, unsigned char *data, size_t length);
  		hid_device*  hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number=NULL);
		int  hid_get_manufacturer_string(hid_device *device, wchar_t *string, size_t maxlen);
		int  hid_get_product_string(hid_device *device, wchar_t *string, size_t maxlen);
		int  hid_get_serial_number_string(hid_device *device, wchar_t *string, size_t maxlen);
		int  hid_get_indexed_string(hid_device *device, int string_index, wchar_t *string, size_t maxlen);
};

#endif // HID_API_H





