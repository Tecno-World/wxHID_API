/******************************************************************************
 * Name:      hid_api_win.cpp
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

#include "hid_api.h"
#include <windows.h>

#include <ntdef.h>
#include <winbase.h>
#include <setupapi.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wx/log.h"


/* The maximum number of characters that can be passed into the
   HidD_Get*String() functions without it failing.*/
#define MAX_STRING_WCHARS 0xFFF

#undef MIN
#define MIN(x,y) ((x) < (y)? (x): (y))


//***********************************************************************************************************
HID_API::HidD_GetAttributes_ HID_API::HidD_GetAttributes=0;
HID_API::HidD_GetSerialNumberString_ HID_API::HidD_GetSerialNumberString=0;
HID_API::HidD_GetManufacturerString_ HID_API::HidD_GetManufacturerString=0;
HID_API::HidD_GetProductString_ HID_API::HidD_GetProductString=0;
HID_API::HidD_SetFeature_ HID_API::HidD_SetFeature=0;
HID_API::HidD_GetFeature_ HID_API::HidD_GetFeature=0;
HID_API::HidD_GetIndexedString_ HID_API::HidD_GetIndexedString=0;
HID_API::HidD_GetPreparsedData_ HID_API::HidD_GetPreparsedData=0;
HID_API::HidD_FreePreparsedData_ HID_API::HidD_FreePreparsedData=0;
HID_API::HidD_SetNumInputBuffers_ HID_API::HidD_SetNumInputBuffers=0;
HID_API::HidP_GetCaps_ HID_API::HidP_GetCaps=0;
//neu...
HID_API::HidD_GetInputReport_ HID_API::HidD_GetInputReport=0;
HID_API::HidD_SetOutputReport_ HID_API::HidD_SetOutputReport=0;

#ifdef __COMPLETE__
HID_API::HidD_GetHidGuid_ HID_API::HidD_GetHidGuid=0;
HID_API::HidD_GetPhysicalDescriptor_ HID_API::HidD_GetPhysicalDescriptor=0;
HID_API::HidD_GetNumInputBuffers_ HID_API::HidD_GetNumInputBuffers=0;
HID_API::HidD_FlushQueue_ HID_API::HidD_FlushQueue;
HID_API::HidD_GetConfiguration_ HID_API::HidD_GetConfiguration;
HID_API::HidD_GetMsGenreDescriptor_ HID_API::HidD_GetMsGenreDescriptor;
HID_API::HidD_Hello_ HID_API::HidD_Hello;
HID_API::HidD_SetConfiguration_ HID_API::HidD_SetConfiguration;

HID_API::HidP_GetButtonCaps_ HID_API::HidP_GetButtonCaps;
HID_API::HidP_GetData_ HID_API::HidP_GetData;
HID_API::HidP_GetExtendedAttributes_ HID_API::HidP_GetExtendedAttributes;
HID_API::HidP_GetLinkCollectionNodes_ HID_API::HidP_GetLinkCollectionNodes;
HID_API::HidP_GetScaledUsageValue_ HID_API::HidP_GetScaledUsageValue;
HID_API::HidP_GetSpecificButtonCaps_ HID_API::HidP_GetSpecificButtonCaps;
HID_API::HidP_GetSpecificValueCaps_ HID_API::HidP_GetSpecificValueCaps;
HID_API::HidP_GetUsages_ HID_API::HidP_GetUsages;
HID_API::HidP_GetUsagesEx_ HID_API::HidP_GetUsagesEx;
HID_API::HidP_GetUsageValue_ HID_API::HidP_GetUsageValue;
HID_API::HidP_GetUsageValueArray_ HID_API::HidP_GetUsageValueArray;
HID_API::HidP_GetValueCaps_ HID_API::HidP_GetValueCaps;
HID_API::HidP_InitializeReportForID_ HID_API::HidP_InitializeReportForID;
HID_API::HidP_MaxDataListLength_ HID_API::HidP_MaxDataListLength;
HID_API::HidP_MaxUsageListLength_ HID_API::HidP_MaxUsageListLength;
HID_API::HidP_SetData_ HID_API::HidP_SetData;
HID_API::HidP_SetScaledUsageValue_ HID_API::HidP_SetScaledUsageValue;
HID_API::HidP_SetUsages_ HID_API::HidP_SetUsages;
HID_API::HidP_SetUsageValue_ HID_API::HidP_SetUsageValue;
HID_API::HidP_SetUsageValueArray_ HID_API::HidP_SetUsageValueArray;
HID_API::HidP_UnsetUsages_ HID_API::HidP_UnsetUsages;
HID_API::HidP_UsageListDifference_ HID_API::HidP_UsageListDifference;
#endif // __COMPLETE__


HMODULE HID_API::lib_handle = NULL;
BOOLEAN HID_API::initialized = false;
bool HID_API::adv_install=false;
HID_API::hid_device* HID_API::m_device=0;

HID_API::HID_API(PollingEvent_t EvtFunc)
 : wxThread(wxTHREAD_DETACHED)
{
   is_enumerated=false;
   if (EvtFunc) PollEvt=EvtFunc;
   if(wxTHREAD_NO_ERROR == Create()) {
      Run();
    }
}

void HID_API::hid_device::Clear()
{
	device_handle = INVALID_HANDLE_VALUE;
	blocking = TRUE;
	output_report_length = 0;
	input_report_length = 0;
	last_error_str = wxEmptyString;
	last_error_num = 0;
	read_pending = FALSE;
	memset(&ol, 0, sizeof(ol));
	memset(read_buf, 0, sizeof(read_buf));
	memset(out_buf, 0, sizeof(out_buf));
	memset(feat_buf, 0, sizeof(feat_buf));
	ol.hEvent = CreateEvent(NULL, FALSE, FALSE /*initial state f=nonsignaled*/, NULL);
}


void* HID_API::Entry()
{
   hid_init();
   IntReadEnabled=false;
   int bytesRead;
   unsigned char buff[512];
   while (1)
   {
        if (IntReadEnabled && m_device)
        {
            bytesRead=hid_read_timeout(m_device,buff,sizeof(buff),10);
            if (bytesRead) PollEvt(buff,bytesRead);
        } else wxMilliSleep(20);
   }
   return 0;
}

void HID_API::EnablePolling(bool enable)
{
    IntReadEnabled = enable && m_device;
}


HID_API::~HID_API()
{
	if (m_device) free_hid_device(m_device);
	if (lib_handle)	FreeLibrary(lib_handle);
	lib_handle = NULL;
	initialized = FALSE;
}


void HID_API::free_hid_device(hid_device *dev)
{
	CloseHandle(dev->ol.hEvent);
	CloseHandle(dev->device_handle);
	//delete dev->read_buf;
    delete dev;
    dev = 0;
}

void HID_API::register_error(hid_device *device, wxString op)
{
	WCHAR *ptr, *msg;

	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
        (LPWSTR)&msg,
        0/*sz*/,
        NULL);

	/* Get rid of the CR and LF that FormatMessage() sticks at the
	   end of the message. Thanks Microsoft! */
	ptr = msg;
	while (*ptr) {
		if (*ptr == '\r') {
			*ptr = 0x0000;
			break;
		}
		ptr++;
	}
	/* Store the message off in the Device entry so that
	   the hid_error() function can pick it up. */
	device->last_error_str = wxString(msg);
}

int HID_API::lookup_functions()
{
	lib_handle = LoadLibraryA("hid.dll");
	adv_install=true;
	if (lib_handle) {
#define RESOLVE(x) x = (x##_)GetProcAddress(lib_handle, #x); if (!x) return -1;
		RESOLVE(HidD_GetAttributes);
		RESOLVE(HidD_GetSerialNumberString);
		RESOLVE(HidD_GetManufacturerString);
		RESOLVE(HidD_GetProductString);
		RESOLVE(HidD_SetFeature);
		RESOLVE(HidD_GetFeature);
		RESOLVE(HidD_GetIndexedString);
		RESOLVE(HidD_GetPreparsedData);
		RESOLVE(HidD_FreePreparsedData);
		RESOLVE(HidD_SetNumInputBuffers);
		RESOLVE(HidP_GetCaps);
        //neu....
        RESOLVE(HidD_GetInputReport);
        RESOLVE(HidD_SetOutputReport);

#ifdef __COMPLETE__
#undef RESOLVE
#define RESOLVE(x) x = (x##_)GetProcAddress(lib_handle, #x); if (!x) adv_install=false;
        RESOLVE(HidD_FlushQueue);
        RESOLVE(HidD_GetConfiguration);
        RESOLVE(HidD_GetMsGenreDescriptor);
        RESOLVE(HidD_Hello);
        RESOLVE(HidD_SetConfiguration);


        RESOLVE(HidD_GetHidGuid);
        RESOLVE(HidD_GetPhysicalDescriptor);
        RESOLVE(HidD_GetNumInputBuffers);
        RESOLVE(  HidP_GetButtonCaps);
        RESOLVE(  HidP_GetData);
        RESOLVE(  HidP_GetExtendedAttributes);
        RESOLVE(  HidP_GetLinkCollectionNodes);
        RESOLVE(  HidP_GetScaledUsageValue);
        RESOLVE(  HidP_GetSpecificButtonCaps);
        RESOLVE(  HidP_GetSpecificValueCaps);
        RESOLVE(  HidP_GetUsages);
        RESOLVE(  HidP_GetUsagesEx);
        RESOLVE(  HidP_GetUsageValue);
        RESOLVE(  HidP_GetUsageValueArray);
        RESOLVE(  HidP_GetValueCaps);
        RESOLVE(  HidP_InitializeReportForID);
        RESOLVE(  HidP_MaxDataListLength);
        RESOLVE(  HidP_MaxUsageListLength);
        RESOLVE(  HidP_SetData);
        RESOLVE(  HidP_SetScaledUsageValue);
        RESOLVE(  HidP_SetUsages);
        RESOLVE(  HidP_SetUsageValue);
        RESOLVE(  HidP_SetUsageValueArray);
        RESOLVE(  HidP_UnsetUsages);
        RESOLVE(  HidP_UsageListDifference);
#endif // __COMPLETE__
#undef RESOLVE
	}
	else
		return -1;

	return 0;
}


int HID_API::hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	BOOL res;
	hid_device_info_t cur_dev;
    device_list.clear();

	/* Windows objects for interacting with the driver. */
	GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };
	SP_DEVINFO_DATA devinfo_data;
	SP_DEVICE_INTERFACE_DATA device_interface_data;
	SP_DEVICE_INTERFACE_DETAIL_DATA_A *device_interface_detail_data = NULL;
	HDEVINFO device_info_set = INVALID_HANDLE_VALUE;
	int device_index = 0;
	int i;

	if (hid_init() < 0)
    {
		return 0;
    }

	/* Initialize the Windows objects. */
	memset(&devinfo_data, 0x0, sizeof(devinfo_data));
	devinfo_data.cbSize = sizeof(SP_DEVINFO_DATA);
	device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	/* Get information for all the devices belonging to the HID class. */
	device_info_set = SetupDiGetClassDevsA(&InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	/* Iterate over each device in the HID class, looking for the right one. */

	for (;;)
    {
		HANDLE write_handle = INVALID_HANDLE_VALUE;
		DWORD required_size = 0;
		HIDD_ATTRIBUTES_t attrib;

		res = SetupDiEnumDeviceInterfaces(device_info_set,NULL,	&InterfaceClassGuid,device_index,&device_interface_data);
		if (!res) {break;}
			/* A return of FALSE from this function means that
			   there are no more devices. */

		/* Call with 0-sized detail size, and let the function
		   tell us how long the detail struct needs to be. The
		   size is put in &required_size. */
		res = SetupDiGetDeviceInterfaceDetailA(device_info_set,	&device_interface_data,	NULL,0,	&required_size,	NULL);
		/* Allocate a long enough structure for device_interface_detail_data. */
		device_interface_detail_data = new SP_DEVICE_INTERFACE_DETAIL_DATA_A[required_size];
		device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

		/* Get the detailed data for this device. The detail data gives us
		   the device path for this device, which is then passed into
		   CreateFile() to get a handle to the device. */
		res = SetupDiGetDeviceInterfaceDetailA(device_info_set,	&device_interface_data,	device_interface_detail_data,required_size,	NULL,NULL);
		if (!res) break;  // register_error(dev, "Unable to call SetupDiGetDeviceInterfaceDetail");
		/* Make sure this device is of Setup Class "HIDClass" and has a
		   driver bound to it. */
		for (i = 0; ; i++) {
			char driver_name[256];

			/* Populate devinfo_data. This function will return failure
			   when there are no more interfaces left. */
			res = SetupDiEnumDeviceInfo(device_info_set, i, &devinfo_data);
			if (!res) break;
			res = SetupDiGetDeviceRegistryPropertyA(device_info_set, &devinfo_data, SPDRP_CLASS, NULL, (PBYTE)driver_name, sizeof(driver_name), NULL);
			if (!res) break;
			if (strcmp(driver_name, "HIDClass") == 0) {
				/* See if there's a driver bound. */
				res = SetupDiGetDeviceRegistryPropertyA(device_info_set, &devinfo_data, SPDRP_DRIVER, NULL, (PBYTE)driver_name, sizeof(driver_name), NULL);
				if (res) break;
			}
		}
		//wprintf(L"HandleName: %s\n", device_interface_detail_data->DevicePath);
		/* Open a handle to the device */
		write_handle = open_device(device_interface_detail_data->DevicePath, TRUE);

		/* Check validity of write_handle. */
		if (write_handle != INVALID_HANDLE_VALUE)
        {
			/* Unable to open the device. */
			//register_error(dev, "CreateFile");
			//goto cont_close;
            /* Get the Vendor ID and Product ID for this device. */
            attrib.Size = sizeof(HIDD_ATTRIBUTES_t);
            HidD_GetAttributes(write_handle, &attrib);
            //wprintf(L"Product/Vendor: %x %x\n", attrib.ProductID, attrib.VendorID);
            /* Check the VID/PID to see if we should add this
               device to the enumeration list. */
            if ((vendor_id == 0x0 || attrib.VendorID == vendor_id) &&
                (product_id == 0x0 || attrib.ProductID == product_id)) {

                #define WSTR_LEN 512
                const char* str;

                PHIDP_PREPARSED_DATA pp_data = NULL;
                HIDP_CAPS_t caps;
                BOOLEAN res;
                NTSTATUS nt_res;
                wchar_t wstr[WSTR_LEN]; /* TODO: Determine Size */

                /* VID/PID match. Create the record. */
                /* Get the Usage Page and Usage for this device. */
                res = HidD_GetPreparsedData(write_handle, &pp_data);
                if (res)
                {
                    nt_res = HidP_GetCaps(pp_data, &caps);
                    if (nt_res == HIDP_STATUS_SUCCESS)
                    {
                        cur_dev.usage_page = caps.UsagePage;
                        cur_dev.usage = caps.Usage;
                    }
                    HidD_FreePreparsedData(pp_data);
                }

                /* Fill out the record */
                str = device_interface_detail_data->DevicePath;
                if (str) cur_dev.m_path = wxString(str);
                  else cur_dev.m_path = wxEmptyString;
                /* Serial Number */
                res = HidD_GetSerialNumberString(write_handle, wstr, sizeof(wstr));
                wstr[WSTR_LEN-1] = 0x0000;
                if (res) cur_dev.serial_number = wxString(wstr);
                /* Manufacturer String */
                res = HidD_GetManufacturerString(write_handle, wstr, sizeof(wstr));
                wstr[WSTR_LEN-1] = 0x0000;
                if (res) cur_dev.manufacturer_string = wxString(wstr);
                /* Product String */
                res = HidD_GetProductString(write_handle, wstr, sizeof(wstr));
                wstr[WSTR_LEN-1] = 0x0000;
                if (res) cur_dev.product_string = wxString(wstr);
                /* VID/PID */
                cur_dev.vendor_id = attrib.VendorID;
                cur_dev.product_id = attrib.ProductID;
                /* Release Number */
                cur_dev.release_number = attrib.VersionNumber;
                /* Interface Number. It can sometimes be parsed out of the path
                   on Windows if a device has multiple interfaces. See
                   http://msdn.microsoft.com/en-us/windows/hardware/gg487473 or
                   search for "Hardware IDs for HID Devices" at MSDN. If it's not
                   in the path, it's set to -1. */
                cur_dev.interface_number = -1;
                if (!cur_dev.m_path.IsEmpty())
                {
                    char *interface_component = strstr(cur_dev.m_path.mb_str(), "&mi_");
                    if (interface_component)
                    {
                        char *hex_str = interface_component + 4;
                        char *endptr = NULL;
                        cur_dev.interface_number = strtol(hex_str, &endptr, 16);
                        if (endptr == hex_str) cur_dev.interface_number = -1; /* The parsing failed. Set interface_number to -1. */
                    }
                }
                device_list.push_back(cur_dev);
            }
        }
		if (write_handle != INVALID_HANDLE_VALUE) CloseHandle(write_handle);
		write_handle=INVALID_HANDLE_VALUE;
		/* We no longer need the detail data. It can be freed */
		delete device_interface_detail_data;
        device_interface_detail_data=0;
		device_index++;
	}
	/* Close the device information handle. */
	SetupDiDestroyDeviceInfoList(device_info_set);
	return device_list.size();
}

HANDLE HID_API::open_device(const char *path, BOOL enumerate)
{
	HANDLE handle;
	DWORD desired_access = (enumerate)? 0: (GENERIC_WRITE | GENERIC_READ);
	DWORD share_mode = FILE_SHARE_READ|FILE_SHARE_WRITE;

	handle = CreateFileA(path,
		desired_access,
		share_mode,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,/*FILE_ATTRIBUTE_NORMAL,*/
		0);

	return handle;
}

int HID_API::hid_init(void)
{
	if (!initialized)
    {
		if (lookup_functions() < 0) return -1;
		initialized = TRUE;
		hid_enumerate();
		is_enumerated=true;
	}
	return 0;
}


int HID_API::GetFirstDevice(hid_device_info_t &Info)
{
    NextDeviceIdx=1;
    return GetNextDevice(Info);
}
int HID_API::GetNextDevice(hid_device_info_t &Info)
{
    if ((NextDeviceIdx<1) || (NextDeviceIdx> device_list.size())) return -1;
    Info=device_list[NextDeviceIdx-1];
    return NextDeviceIdx;
}

HID_API::hid_device*  HID_API::hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
	struct hid_device_info_t* cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;

	for (int a=0; a<device_list.size(); a++)
    {
        cur_dev = &device_list[a];
		if (cur_dev->vendor_id == vendor_id &&
		    cur_dev->product_id == product_id) {
			if (serial_number) {
				if (wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->m_path.mb_str();
					break;
				}
			}
			else {
				path_to_open = cur_dev->m_path.mb_str();
				break;
			}
		}
	}
	if (path_to_open) {
		/* Open the device */
		handle = hid_open_path(path_to_open);
	}
	return handle;
}

bool HID_API::OpenDevice(const char *path)
{
    hid_open_path(path);
    return m_device != 0;
}

HID_API::hid_device*  HID_API::hid_open_path(const char *path)
{
	hid_device *dev;
	HIDP_CAPS_t caps;
	PHIDP_PREPARSED_DATA pp_data = NULL;
	BOOLEAN res;
	NTSTATUS nt_res;
	m_device=0;

	if (hid_init() < 0) {
		return NULL;
	}

	dev = new hid_device();
	dev->Clear();
	dev->device_handle = open_device(path, FALSE);
	if (dev->device_handle == INVALID_HANDLE_VALUE)
    {
		register_error(dev, "CreateFile");
		goto err;
	}
	/* Set the Input Report buffer size to 64 reports. */
	res = HidD_SetNumInputBuffers(dev->device_handle, 64);
	if (!res)
	{
		register_error(dev, "HidD_SetNumInputBuffers");
		goto err;
	}
	/* Get the Input Report length for the device. */
    // Todo: we have more as one Input Report
	res = HidD_GetPreparsedData(dev->device_handle, &pp_data);
	if (!res) {
		register_error(dev, "HidD_GetPreparsedData");
		goto err;
	}
	nt_res = HidP_GetCaps(pp_data, &caps);
	if (nt_res != HIDP_STATUS_SUCCESS) {
		register_error(dev, "HidP_GetCaps");
		goto err_pp_data;
	}
	dev->output_report_length = caps.OutputReportByteLength;
	dev->input_report_length = caps.InputReportByteLength;
	HidD_FreePreparsedData(pp_data);

    m_device=dev;
	return dev;

err_pp_data:
		HidD_FreePreparsedData(pp_data);
err:
		free_hid_device(dev);
		return NULL;
}

int  HID_API::hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	DWORD bytes_written;
	BOOL res;

	OVERLAPPED ol;
	unsigned char *buf;
	memset(&ol, 0, sizeof(ol));

	/* Make sure the right number of bytes are passed to WriteFile. Windows
	   expects the number of bytes which are in the _longest_ report (plus
	   one for the report number) bytes even if the data is a report
	   which is shorter than that. Windows gives us this value in
	   caps.OutputReportByteLength. If a user passes in fewer bytes than this,
	   create a temporary buffer which is the proper size. */
	//Todo: we have more as one Output report.
	if (length >= dev->output_report_length)
    {
		/* The user passed the right number of bytes. Use the buffer as-is. */
		buf = (unsigned char *) data;
	} else
	{
		/* Create a temporary buffer and copy the user's data
		   into it, padding the rest with zeros. */
		buf = new unsigned char[dev->output_report_length];
		memcpy(buf, data, length);
		memset(buf + length, 0, dev->output_report_length - length);
		length = dev->output_report_length;
	}
	res = WriteFile(dev->device_handle, buf, length, NULL, &ol);
	if (!res)
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			/* WriteFile() failed. Return error. */
			register_error(dev, "WriteFile");
			bytes_written = -1;
			goto end_of_function;
		}
	}
	/* Wait here until the write is done. This makes
	   hid_write() synchronous. */
	res = GetOverlappedResult(dev->device_handle, &ol, &bytes_written, TRUE/*wait*/);
	if (!res)
	{
		/* The Write operation failed. */
		register_error(dev, "WriteFile");
		bytes_written = -1;
		goto end_of_function;
	}

end_of_function:
	if (buf != data)
    {
        delete buf;
        buf=0;
    }
	return bytes_written;
}


int  HID_API::hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
	DWORD bytes_read = 0;
	size_t copy_len = 0;
	BOOL res;

	/* Copy the handle for convenience. */
	HANDLE ev = dev->ol.hEvent;

	if (!dev->read_pending) {
		/* Start an Overlapped I/O read. */
		dev->read_pending = TRUE;
		memset(dev->read_buf, 0, dev->input_report_length);
		ResetEvent(ev);
		res = ReadFile(dev->device_handle, dev->read_buf, dev->input_report_length, &bytes_read, &dev->ol);
		if (!res)
        {
			if (GetLastError() != ERROR_IO_PENDING)
			{
				/* ReadFile() has failed.
				   Clean up and return error. */
				CancelIo(dev->device_handle);
				dev->read_pending = FALSE;
				goto end_of_function;
			}
		}
	}
	if (milliseconds >= 0) {
		/* See if there is any data yet. */
		res = WaitForSingleObject(ev, milliseconds);
		if (res != WAIT_OBJECT_0)
        {
			/* There was no data this time. Return zero bytes available,
			   but leave the Overlapped I/O running. */
			return 0;
		}
	}

	/* Either WaitForSingleObject() told us that ReadFile has completed, or
	   we are in non-blocking mode. Get the number of bytes read. The actual
	   data has been copied to the data[] array which was passed to ReadFile(). */
	res = GetOverlappedResult(dev->device_handle, &dev->ol, &bytes_read, TRUE/*wait*/);

	/* Set pending back to false, even if GetOverlappedResult() returned error. */
	dev->read_pending = FALSE;

	if (res && bytes_read > 0)
    {
		#if (0) //todo: wenn multiple Input reports vorhanden, was dann? Byte 0 muss immer ID sein...
        {
            if (dev->read_buf[0] == 0x0)
            {
                /* If report numbers aren't being used, but Windows sticks a report
                   number (0x0) on the beginning of the report anyway. To make this
                   work like the other platforms, and to make it work more like the
                   HID spec, we'll skip over this byte. */
                bytes_read--;
                copy_len = length > bytes_read ? bytes_read : length;
                memcpy(data, dev->read_buf+1, copy_len);
            }
        }
		else
        #endif
		{
			/* Copy the whole buffer, report number and all. */
			copy_len = length > bytes_read ? bytes_read : length;
			memcpy(data, dev->read_buf, copy_len);
		}
	}

end_of_function:
	if (!res)
    {
		register_error(dev, "GetOverlappedResult");
		return -1;
	}
	return copy_len;
}

int  HID_API::hid_read(hid_device *dev, unsigned char *data, size_t length)
{
	return hid_read_timeout(dev, data, length, (dev->blocking)? -1: 0);
}

int  HID_API::hid_set_nonblocking(hid_device *dev, int nonblock)
{
	dev->blocking = !nonblock;
	return 0; /* Success */
}

int  HID_API::hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
	BOOL res = HidD_SetFeature(dev->device_handle, (PVOID)data, length);
	if (!res)
    {
		register_error(dev, "HidD_SetFeature");
		return -1;
	}
	return length;
}

bool HID_API::GetInputReport(char* buffer, size_t len)
{
    bool res = HidD_GetInputReport(m_device->device_handle,(PVOID)buffer, len);
    return res;
}


int  HID_API::hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	BOOL res;
#if 1
	res = HidD_GetFeature(dev->device_handle, data, length);
	if (!res) {
		register_error(dev, "HidD_GetFeature");
		return -1;
	}
	return 0; /* HidD_GetFeature() doesn't give us an actual length, unfortunately */
#else //funktioniert nur bei Keyboard....
	DWORD bytes_returned;

	OVERLAPPED ol;
	memset(&ol, 0, sizeof(ol));

	res = DeviceIoControl(dev->device_handle,
		IOCTL_HID_GET_FEATURE,
		data, length,
		data, length,
		&bytes_returned, &ol);

	if (!res) {
		if (GetLastError() != ERROR_IO_PENDING) {
			/* DeviceIoControl() failed. Return error. */
			register_error(dev, "Send Feature Report DeviceIoControl");
			return -1;
		}
	}

	/* Wait here until the write is done. This makes
	   hid_get_feature_report() synchronous. */
	res = GetOverlappedResult(dev->device_handle, &ol, &bytes_returned, TRUE/*wait*/);
	if (!res) {
		/* The operation failed. */
		register_error(dev, "Send Feature Report GetOverLappedResult");
		return -1;
	}

	/* bytes_returned does not include the first byte which contains the
	   report ID. The data buffer actually contains one more byte than
	   bytes_returned. */
	bytes_returned++;

	return bytes_returned;
#endif
}

void HID_API::CloseDevice()
{
    hid_close(m_device);
}

void  HID_API::hid_close(hid_device *dev)
{
	if (!dev) return;
	CancelIo(dev->device_handle);
	free_hid_device(dev);
	m_device=0;
}

int  HID_API::hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetManufacturerString(dev->device_handle, string, sizeof(wchar_t) * MIN(maxlen, MAX_STRING_WCHARS));
	if (!res) {
		register_error(dev, "HidD_GetManufacturerString");
		return -1;
	}
	return 0;
}

int  HID_API::hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetProductString(dev->device_handle, string, sizeof(wchar_t) * MIN(maxlen, MAX_STRING_WCHARS));
	if (!res) {
		register_error(dev, "HidD_GetProductString");
		return -1;
	}

	return 0;
}

int  HID_API::hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetSerialNumberString(dev->device_handle, string, sizeof(wchar_t) * MIN(maxlen, MAX_STRING_WCHARS));
	if (!res) {
		register_error(dev, "HidD_GetSerialNumberString");
		return -1;
	}

	return 0;
}

int  HID_API::hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetIndexedString(dev->device_handle, string_index, string, sizeof(wchar_t) * MIN(maxlen, MAX_STRING_WCHARS));
	if (!res) {
		register_error(dev, "HidD_GetIndexedString");
		return -1;
	}

	return 0;
}


const wxString HID_API::hid_error(hid_device *dev)
{
	if (dev) return dev->last_error_str;
	 else return wxEmptyString;
}

