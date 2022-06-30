# define the arguments you want to submit to the method
# remove values that you do not want to submit
# make sure you replace values with meaningful content before running the code
# see section "Parameters" below for a description of each argument.
$arguments = @{
    TcpMaxDataRetransmissions = [UInt32](12345)  # replace 12345 with a meaningful value
}

Invoke-CimMethod -ClassName Win32_NetworkAdapterConfiguration -Namespace Root/CIMV2 -MethodName SetTcpMaxDataRetransmissions -Arguments $arguments |
Add-Member -MemberType ScriptProperty -Name ReturnValueFriendly -Passthru -Value {
  switch ([int]$this.ReturnValue)
  {
        0        {'Successful completion, no reboot required'}
        1        {'Successful completion, reboot required'}
        64       {'Method not supported on this platform'}
        65       {'Unknown failure'}
        66       {'Invalid subnet mask'}
        67       {'An error occurred while processing an Instance that was returned'}
        68       {'Invalid input parameter'}
        69       {'More than 5 gateways specified'}
        70       {'Invalid IP  address'}
        71       {'Invalid gateway IP address'}
        72       {'An error occurred while accessing the Registry for the requested information'}
        73       {'Invalid domain name'}
        74       {'Invalid host name'}
        75       {'No primary/secondary WINS server defined'}
        76       {'Invalid file'}
        77       {'Invalid system path'}
        78       {'File copy failed'}
        79       {'Invalid security parameter'}
        80       {'Unable to configure TCP/IP service'}
        81       {'Unable to configure DHCP service'}
        82       {'Unable to renew DHCP lease'}
        83       {'Unable to release DHCP lease'}
        84       {'IP not enabled on adapter'}
        85       {'IPX not enabled on adapter'}
        86       {'Frame/network number bounds error'}
        87       {'Invalid frame type'}
        88       {'Invalid network number'}
        89       {'Duplicate network number'}
        90       {'Parameter out of bounds'}
        91       {'Access denied'}
        92       {'Out of memory'}
        93       {'Already exists'}
        94       {'Path, file or object not found'}
        95       {'Unable to notify service'}
        96       {'Unable to notify DNS service'}
        97       {'Interface not configurable'}
        98       {'Not all DHCP leases could be released/renewed'}
        100      {'DHCP not enabled on adapter'}
        default  {'Unknown Error '}
    }
}