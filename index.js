const addon = require('./build/Release/vddk.node')

class Vddk {
    constructor() {
        this._wrapper = new addon.VddkWrapper();
    }

    async connect(server, thumbprint, username, password) {
        return this._wrapper.connect(server, thumbprint, username, password);
    }

    async disconnect() {
        return this._wrapper.disconnect();
    }

    async openDisk(path) {
        return this._wrapper.openDisk(path);
    }

    async closeDisk() {
        return this._wrapper.closeDisk();
    }

    async read(offset, length) {
        return this._wrapper.read(offset, length);
    }

    async write(offset, buffer) {
        return this._wrapper.write(offset, buffer);
    }
}

module.exports = Vddk;