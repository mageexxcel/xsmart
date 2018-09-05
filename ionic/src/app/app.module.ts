import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { HttpClientModule } from '@angular/common/http';
import { RouterModule, RouteReuseStrategy } from '@angular/router';

import { IonicModule, IonicRouteStrategy, Platform } from '@ionic/angular';
import { SplashScreen } from '@ionic-native/splash-screen/ngx';
import { NativeStorage } from '@ionic-native/native-storage/ngx';
import { UniqueDeviceID } from '@ionic-native/unique-device-id/ngx';
import { StatusBar } from '@ionic-native/status-bar/ngx';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { LoginComponent } from './login/login.component';
import { RegisterComponent } from './register/register.component';
import { ScanDevicesComponent } from './scan-devices/scan-devices.component';
import { PairDevicesComponent } from './pair-devices/pair-devices.component';
import { AddDevicesComponent } from './add-devices/add-devices.component';
import { ApiService } from './api/api.service';
import { DeviceService } from './api/device.service';

@NgModule({
  declarations: [
    AppComponent,
    LoginComponent,
    RegisterComponent,
    ScanDevicesComponent,
    PairDevicesComponent,
    AddDevicesComponent],
  entryComponents: [],
  imports: [BrowserModule,
    HttpClientModule,
    FormsModule,

    ReactiveFormsModule,
    IonicModule.forRoot(),
    AppRoutingModule],
  providers: [
    StatusBar,
    SplashScreen,
    NativeStorage,
    UniqueDeviceID,
    { provide: RouteReuseStrategy, useClass: IonicRouteStrategy }
  ],
  bootstrap: [AppComponent]
})
export class AppModule {
  deviceid: string;
  constructor(public deviceServices: DeviceService,
    public pltform: Platform,
    private nativeStorage: NativeStorage
  ) {
    this.deviceId();
   
    
  }
  deviceId() {
    if (this.pltform.is('cordova')) {
      this.deviceServices.getAppID().then((res)=>{
        this.nativeStorage.setItem('id', res)
      })
    }
  }
}