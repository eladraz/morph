using System;
using System.Collections.Generic;
using System.Text;

namespace Morph.SimpleLink
{
    public class CC3000_API
    {
        [clrcore.Import("_wlan_init_wrapper"), clrcore.CallingConvention("cdecl")]
        public unsafe static void wlan_init()
        {
        }

        [clrcore.Import("_wlan_start"), clrcore.CallingConvention("cdecl")]
        public unsafe static void wlan_start(ushort usPatchesAvailableAtHost) { }

        [clrcore.Import("_wlan_stop"), clrcore.CallingConvention("cdecl")]
        public unsafe static void wlan_stop() { }

        [clrcore.Import("_wlan_connect"), clrcore.CallingConvention("cdecl")]
        public unsafe static long wlan_connect(ulong ulSecType, char* ssid, long ssid_len,
            byte* bssid, byte* key, long key_len) { return 0; }

        [clrcore.Import("_wlan_is_connected"), clrcore.CallingConvention("cdecl")]
        public unsafe static ulong wlan_is_connected()
        {
            return 0;
        }

        [clrcore.Import("_wlan_is_dhcp"), clrcore.CallingConvention("cdecl")]
        public unsafe static ulong wlan_is_dhcp()
        {
            return 0;
        }

        [clrcore.Import("_wlan_disconnect"), clrcore.CallingConvention("cdecl")]
        public unsafe static long wlan_disconnect() { return 0; }

        //[clrcore.Import("_wlan_add_profile"), clrcore.CallingConvention("cdecl")]
        //public unsafe static long wlan_add_profile(ulong ulSecType, char* ucSsid,
        //                                         ulong ulSsidLen,
        //                                         char* ucBssid,
        //                                         ulong ulPriority,
        //                                         ulong ulPairwiseCipher_Or_Key,
        //                                         ulong ulGroupCipher_TxKeyLen,
        //                                         ulong ulKeyMgmt,
        //                                         char* ucPf_OrKey,
        //                                         ulong ulPassPhraseLen) { return 0; }


        //[clrcore.Import("_wlan_ioctl_del_profile"), clrcore.CallingConvention("cdecl")]
        //public unsafe static long wlan_ioctl_del_profile(ulong ulIndex) { return 0; }

        //[clrcore.Import("_wlan_set_event_mask"), clrcore.CallingConvention("cdecl")]
        //public unsafe static long wlan_set_event_mask(ulong ulMask) { return 0; }

        [clrcore.Import("_wlan_ioctl_statusget"), clrcore.CallingConvention("cdecl")]
        public unsafe static long wlan_ioctl_statusget() { return 0; }

        //[clrcore.Import("_wlan_ioctl_set_connection_policy"), clrcore.CallingConvention("cdecl")]
        //public unsafe static long wlan_ioctl_set_connection_policy(
        //                                        ulong should_connect_to_open_ap,
        //                                        ulong should_use_fast_connect,
        //                                        ulong ulUseProfiles) { return 0; }

        [clrcore.Import("_wlan_ioctl_get_scan_results"), clrcore.CallingConvention("cdecl")]
        public unsafe static long wlan_ioctl_get_scan_results(ulong ulScanTimeout,
                                               char* ucResults) { return 0; }

        [clrcore.Import("_wlan_ioctl_set_scan_params"), clrcore.CallingConvention("cdecl")]
        public unsafe static long wlan_ioctl_set_scan_params(ulong uiEnable, ulong uiMinDwellTime, ulong uiMaxDwellTime,
                                                   ulong uiNumOfProbeResponces, ulong uiChannelMask,
                                                   long iRSSIThreshold, ulong uiSNRThreshold,
                                                   ulong uiDefaultTxPower, ulong* aiIntervalList) { return 0; }

        //[clrcore.Import("_wlan_first_time_config_start"), clrcore.CallingConvention("cdecl")]
        //public unsafe static long wlan_first_time_config_start() { return 0; }

        //[clrcore.Import("_wlan_first_time_config_stop"), clrcore.CallingConvention("cdecl")]
        //public unsafe static long wlan_first_time_config_stop() { return 0; }

        //[clrcore.Import("_wlan_first_time_config_set_prefix"), clrcore.CallingConvention("cdecl")]
        //public unsafe static long wlan_first_time_config_set_prefix(char* cNewPrefix) { return 0; }

    }

    public class CC3000_Socket_API
    {
        public struct sockaddr
        {
            public ushort sa_family;
            public unsafe fixed char sa_data[14];
        }

        public struct sockaddr_in
        {
            public short sin_family;            // e.g. AF_INET
            public ushort sin_port;              // e.g. htons(3490)
            public ulong sin_addr;              // see struct in_addr, below
            public unsafe fixed char sin_zero[8];           // zero this if you want to
        }

        public struct fd_set
        {
            public unsafe fixed long fds_bits[1];   //__FD_SETSIZE / __NFDBITS = 1
        }

        public struct timeval
        {
            public long tv_sec;                  /* seconds */
            public long tv_usec;                 /* microseconds */
        }

        [clrcore.Import("_socket"), clrcore.CallingConvention("cdecl")]
        public unsafe static int socket(long domain, long type, long protocol)
        {
            return 0;
        }

        [clrcore.Import("_closesocket"), clrcore.CallingConvention("cdecl")]
        public unsafe static long closesocket(long sd)
        {
            return 0;
        }

        [clrcore.Import("_accept"), clrcore.CallingConvention("cdecl")]
        public unsafe static long accept(long sd, void* addr, ulong* addrlen)
        {
            return 0;
        }

        [clrcore.Import("_bind"), clrcore.CallingConvention("cdecl")]
        public unsafe static long bind(long sd, void* addr, long addrlen)
        {
            return 0;
        }

        [clrcore.Import("_listen"), clrcore.CallingConvention("cdecl")]
        public unsafe static long listen(long sd, long backlog)
        {
            return 0;
        }

        [clrcore.Import("_connect"), clrcore.CallingConvention("cdecl")]
        public unsafe static long connect(long sd, void* addr, long addrlen)
        {
            return 0;
        }

        [clrcore.Import("_select"), clrcore.CallingConvention("cdecl")]
        public unsafe static int select(long nfds, void* readsds, void* writesds,
                         void* exceptsds, void* timeout)
        {
            return 0;
        }

        [clrcore.Import("_setsockopt"), clrcore.CallingConvention("cdecl")]
        public unsafe static int setsockopt(long sd, long level, long optname, void* optval,
                             ulong optlen)
        {
            return 0;
        }

        [clrcore.Import("_getsockopt"), clrcore.CallingConvention("cdecl")]
        public unsafe static int getsockopt(long sd, long level, long optname, void* optval,
                             ulong* optlen)
        {
            return 0;
        }

        [clrcore.Import("_recv"), clrcore.CallingConvention("cdecl")]
        public unsafe static int recv(long sd, void* buf, long len, long flags)
        {
            return 0;
        }

        [clrcore.Import("_recvfrom"), clrcore.CallingConvention("cdecl")]
        public unsafe static int recvfrom(long sd, void* buf, long len, long flags, sockaddr* from,
                           ulong* fromlen)
        {
            return 0;
        }

        [clrcore.Import("_send"), clrcore.CallingConvention("cdecl")]
        public unsafe static int send(long sd, void* buf, long len, long flags)
        {
            return 0;
        }


        [clrcore.Import("_sendto"), clrcore.CallingConvention("cdecl")]
        public unsafe static int sendto(long sd, void* buf, long len, long flags,
                         void* to, ulong tolen)
        {
            return 0;
        }

        [clrcore.Import("_resolve_hostname"), clrcore.CallingConvention("cdecl")]
        public unsafe static int resolve_hostname(char* hostname, ushort usNameLen, ulong* out_ip_addr)
        {
            return 0;
        }

    }
}