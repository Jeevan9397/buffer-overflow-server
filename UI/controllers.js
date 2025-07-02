// controllers.js
angular.module('bfApp')
.controller('LoginCtrl', function($scope, $location) {
  $scope.user = { user: '', pass: '', showPass: false };

  $scope.togglePass = function() {
    $scope.user.showPass = !$scope.user.showPass;
  };

  $scope.login = function() {
    if ($scope.user.user === 'ravi' && $scope.user.pass === '12345678') {
      $location.path('/dashboard');
    } else {
      $scope.error = "Invalid username or password.";
    }
  };
})
.controller('DashboardCtrl', function($scope, $http, $location) {
  $scope.records = [];
  $scope.newRecord = {};
  $scope.showAddModal = false;

  $http.get('/api/customers').then(function(response) {
    if (response.data && response.data.customers) {
      $scope.records = response.data.customers.map(c => ({
        name: c.name,
        father: c.father,
        nationality: c.nationality,
        dob: c.dob,
        birthplace: `${c.birth_village}, ${c.birth_district}, ${c.birth_province}`,
        address: `${c.street}, ${c.city}, ${c.home_district}, ${c.home_province}`,
        mobile: c.mobile,
        email: c.email
      }));
    }
  });

  $scope.add = function() {
    $scope.newRecord = {};
    $scope.showAddModal = true;
  };

  $scope.cancelAdd = function() {
    $scope.showAddModal = false;
  };

  $scope.saveRecord = function() {
    const parts = ($scope.newRecord.birthplace || '').split(',').map(s => s.trim());
    const addrParts = ($scope.newRecord.address || '').split(',').map(s => s.trim());

    const payload = {
      name: $scope.newRecord.name,
      father: $scope.newRecord.father,
      nationality: $scope.newRecord.nationality,
      dob: $scope.newRecord.dob,
      birth_village: parts[0] || '',
      birth_district: parts[1] || '',
      birth_province: parts[2] || '',
      street: addrParts[0] || '',
      city: addrParts[1] || '',
      home_district: addrParts[2] || '',
      home_province: addrParts[3] || '',
      mobile: $scope.newRecord.mobile,
      email: $scope.newRecord.email
    };

    $http.post('/api/customers', payload).then(function() {
      $scope.records.push(angular.copy($scope.newRecord));
      $scope.showAddModal = false;
      $scope.newRecord = {};
    }, function() {
      alert("Error saving record.");
    });
  };

  $scope.logout = function() {
    $location.path('/login');
  };
});
