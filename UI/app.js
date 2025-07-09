// UI/app.js
angular.module('bfApp', ['ngRoute'])
.config(['$routeProvider', function($routeProvider) {
  $routeProvider
    .when('/login', {
      templateUrl: 'partials/login.html',
      controller:  'LoginCtrl'
    })
    .when('/dashboard', {
      templateUrl: 'partials/dashboard.html',
      controller:  'DashboardCtrl'
    })
    .otherwise({ redirectTo: '/login' });
}]);
