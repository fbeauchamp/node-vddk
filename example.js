const Vddk = require('./index');

async function main() {
    const vddk = new Vddk();
    
    try {
        // Connect to vSphere host
        await vddk.connect(
            '10.10.0.62', 
            'ssl-thumbprint', 
            'root', 
            ''
        );
        
        // Open a VM disk
        await vddk.openDisk('[datastore1] vmname/vmname.vmdk');
        
        // Read 4096 bytes from offset 0
        const data = await vddk.read(0, 4096);
        console.log('Read data:', data);
        
        // Write some data
        const buffer = Buffer.from('Hello VDDK!');
        await vddk.write(0, buffer);
        
        // Clean up
        await vddk.closeDisk();
        await vddk.disconnect();
    } catch (err) {
        console.error('Error:', err);
    }
}

main();