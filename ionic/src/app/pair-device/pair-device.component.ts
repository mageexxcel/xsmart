import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { Ping, Wifi, Device, Switch } from "../api/api"
import { NotifyService } from '../api/notify.service';
import { HttpClient } from '@angular/common/http'
import { ApiService } from '../api/api.service';

@Component({
  selector: 'app-pair-device',
  templateUrl: './pair-device.component.html',
  styleUrls: ['./pair-device.component.scss']
})
export class PairDeviceComponent implements OnInit {

  constructor(
    private router: Router,
    private notifyService: NotifyService,
    private http: HttpClient,
    private api: ApiService,
  ) { }

  ngOnInit() {
    setTimeout(() => {
      this.addDevice();
    }, 1000)
  }
  addDevice() {
    this.router.navigate(["/add-devices"]);
  }

}
