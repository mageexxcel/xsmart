import { Component } from '@angular/core';
import { NativeStorage } from '@ionic-native/native-storage/ngx';
import { Platform } from '@ionic/angular';
import { SplashScreen } from '@ionic-native/splash-screen/ngx';
import { StatusBar } from '@ionic-native/status-bar/ngx';
import { DeviceService } from './api/device.service';

@Component({
  selector: 'app-root',
  templateUrl: 'app.component.html'
})
export class AppComponent {
  constructor(
    private platform: Platform,
    private splashScreen: SplashScreen,
    private statusBar: StatusBar,
    public deviceServices: DeviceService,
    public pltform: Platform,
    private nativeStorage: NativeStorage
  ) {
    this.initializeApp();
    this.deviceId();
  }

  initializeApp() {
    this.platform.ready().then(() => {
      this.statusBar.styleDefault();
      this.splashScreen.hide();
    });
  }
  deviceId() {
    if (this.pltform.is('cordova')) {
      this.deviceServices.getAppID().then((res)=>{
        this.nativeStorage.setItem('id', res)
      })
    }
  }
}
