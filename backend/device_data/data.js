module.exports = {
    Wifi: [{
        ENC: 'encryp',
        RSSI: 100,
        SSID: 'etech1'
    },
    {
        ENC: 'encryp',
        RSSI: 80,
        SSID: 'etech2'
    },
    {
        ENC: 'encryp',
        RSSI: 90,
        SSID: 'etech3'
    }],

    Device: [{
        name: 'fan-switch',
        device_id: 'd1',
        chip: 'chip1',
        ttl: 0,
        online: true,
        WEBID:'W101',
        version:'1.0.2',
        switches: [
            {
                pin: 1,
                status: 1,
                name: "switch1"
            },
            {
                pin: 2,
                status: 0,
                name: "switch2"
            },
            {
                pin: 3,
                status: 1,
                name: "switch3"
            },
            {
                pin: 4,
                status: 1,
                name: "switch4"
            },
        ]
    },
    {
        name: 'bedroom light',
        device_id: 'd2',
        chip: 'chip2',
        ttl: 101,
        online: true,
        WEBID:'W102',
        version:'1.0.3',
        switches: [
            {
                pin: 1,
                status: 1,
                name: "bedroom1"
            },
            {
                pin: 2,
                status: 0,
                name: "bedroom2"
            }
        ]
    },
    {
        name: null,
        device_id: 'd3',
        chip: 'chip3',
        ttl: 101,
        online: true,
        WEBID:'W103',
        version:'1.0.4',
        switches: [
            {
                pin: 1,
                status: 1,
                name: "abcd1"
            },
            {
                pin: 2,
                status: 0,
                name: "abccd2"
            },
            {
                pin: 3,
                status: 1,
                name: "abcd3"
            }
        ]
    },
    {
        name: "laptop",
        device_id: 'd3',
        chip: 'chip4',
        ttl: 101,
        online: true,
        WEBID:'W107',
        version:'1.0.2.7',
        switches: [
            {
                pin: 1,
                status: 1,
                name: "laptop1"
            },
            {
                pin: 2,
                status: 0,
                name: "laptop2"
            },
            {
                pin: 3,
                status: 1,
                name: "laptop3"
            }
        ]
    }
    ]
}